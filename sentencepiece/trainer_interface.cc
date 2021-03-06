// Copyright 2016 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.!

#include "sentencepiece/trainer_interface.h"

#include <cstdlib>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "sentencepiece/model_factory.h"
#include "sentencepiece/model_interface.h"
#include "sentencepiece/normalizer.h"
// #include "sentencepiece/sentencepiece_processor.h"
#include "sentencepiece/unicode_script.h"
#include "sentencepiece/util.h"

namespace sentencepiece {

const char32 TrainerInterface::kWSChar = L'\u2581';
const char TrainerInterface::kWSStr[] = "\xe2\x96\x81";

const char32 TrainerInterface::kUNKChar = L'\u2585';
const char TrainerInterface::kUNKStr[] = "\xe2\x96\x85";

const char32 TrainerInterface::kUPPBoundaryChar = L'\u0009';
const char TrainerInterface::kUPPBoundaryStr[] = "\t";

namespace {
util::Status VerifySpec(const TrainerSpec &trainer_spec) {
  CHECK_OR_RETURN(!trainer_spec.model_prefix().empty());
  CHECK_GT_OR_RETURN(trainer_spec.input().size(), 0);
  CHECK_GT_OR_RETURN(trainer_spec.vocab_size(), 0);

#define CHECK_RANGE(variable, minval, maxval) \
  CHECK_OR_RETURN(variable >= minval && variable <= maxval)

  CHECK_RANGE(trainer_spec.character_coverage(), 0.98, 1.0);
  CHECK_RANGE(trainer_spec.input_sentence_size(), 100, 100000000);
  CHECK_RANGE(trainer_spec.max_sentencepiece_length(), 1, 512);
  CHECK_RANGE(trainer_spec.mining_sentence_size(), 100, 100000000);
  CHECK_RANGE(trainer_spec.num_sub_iterations(), 1, 10);
  CHECK_RANGE(trainer_spec.num_threads(), 1, 128);
  CHECK_RANGE(trainer_spec.seed_sentencepiece_size(), 1000, 5000000);
  CHECK_RANGE(trainer_spec.shrinking_factor(), 0.5, 0.95);
  CHECK_RANGE(trainer_spec.training_sentence_size(), 100, 100000000);
#undef CHECK_RANGE

  return util::OkStatus();
}
}  // namespace

TrainerInterface::TrainerInterface(const TrainerSpec &trainer_spec,
                                   const NormalizerSpec &normalizer_spec)
    : trainer_spec_(trainer_spec), normalizer_spec_(normalizer_spec) {
  status_ = VerifySpec(trainer_spec_);
  if (status_.ok()) status_ = InitMetaPieces();
}

TrainerInterface::~TrainerInterface() {}

bool TrainerInterface::IsValidSentencePiece(
    const string_util::UnicodeText &sentencepiece) const {
  // Returns false if the length of piece is invalid.
  std::string utf8str;
  if (sentencepiece.empty()) {
    return false;
  }
  if (sentencepiece.size() >
      static_cast<size_t>(2 * trainer_spec_.max_sentencepiece_length())) {
    return false;
  }
  utf8str = string_util::UnicodeTextToUTF8(sentencepiece);
  if (sentencepiece.size() >
      static_cast<size_t>(trainer_spec_.max_sentencepiece_length())) {
    if (utf8str.size() >
        static_cast<size_t>(trainer_spec_.max_sentencepiece_length() * 2)) {
      return false;
    }
  }
  if (sentencepiece.size() > 1 && strncmp(utf8str.c_str(), "的", 3) == 0) {
    return false;
  }
  if (sentencepiece.size() > 2 &&
      (strncmp(utf8str.c_str(), "和", 3) == 0 ||
       strncmp(utf8str.c_str() + utf8str.size() - 3, "和", 3) == 0)) {
    return false;
  }
  if (utf8str == "c端" || utf8str == "c++" || utf8str == "c#" ||
      utf8str == "b端" || utf8str == "o2o" || utf8str == "p2p" ||
      utf8str == "b2b" || utf8str == ".net" || utf8str == "b2c" ||
      utf8str == "c2c" || utf8str == "node.js" || utf8str == "4a") {
    return true;
  }

  size_t pos = 0;
  unicode_script::ScriptType prev_script =
      static_cast<unicode_script::ScriptType>(-1);
  for (auto it = sentencepiece.begin(); it != sentencepiece.end(); ++it) {
    if (*it == kUNKChar) {  // UNK must not be included
      return false;
    }
    if (*it == 0x0000) {  // NULL is not allowed for Darts (TRIE).
      return false;
    }
    // kUPPBoundaryChar is included when split_by_upp_for_training is true.
    if (*it == kUPPBoundaryChar) {
      return false;
    }
    if (*it == 0x0020) {
      SPLOG(WARNING) << "space must not be included in normalized string.";
      return false;
    }
    if (!string_util::IsValidCodepoint(*it)) {
      return false;
    }
    if (*it == kWSChar) {
      // Only allows whitespace to appear as a prefix of piece.
      // When split_by_whitespace is false, we allow whitespaces to
      // appear in the middle, "foo_bar", but do not allow them
      // to appear as suffix, "foo_bar_".
      // Regardless of the setting of split_by_whitespace,
      // whitespace is treated as a prefix/infix of symbol or
      // independent symbol.
      if ((trainer_spec_.split_by_whitespace() && pos > 0) ||
          (!trainer_spec_.split_by_whitespace() && pos > 0 &&
           pos == sentencepiece.size() - 1)) {
        return false;
      }
    } else {
      auto s = unicode_script::GetScript(*it);
      // Merge Hiragana/Katakana into Han.
      if (s == unicode_script::U_Hiragana || s == unicode_script::U_Katakana ||
          *it == 0x30FC) {  // long vowel sound (Katakana) should be Katakana
        s = unicode_script::U_Han;
      }
      // Do not allow a piece to include multiple Unicode scripts
      // when split_by_unicode_script() is true (default = true).
      if (prev_script != static_cast<unicode_script::ScriptType>(-1) &&
          prev_script != s && trainer_spec_.split_by_unicode_script()) {
        return false;
      }
      prev_script = s;
    }
    ++pos;
  }
  return true;
}

util::Status TrainerInterface::LoadSentences() {
  RETURN_IF_ERROR(status());
  CHECK_OR_RETURN(sentences_.empty());
  CHECK_OR_RETURN(required_chars_.empty());

  const normalizer::Normalizer normalizer(normalizer_spec_);

  CHECK_OR_RETURN(trainer_spec_.input_format().empty() ||
                  trainer_spec_.input_format() == "text" ||
                  trainer_spec_.input_format() == "tsv")
      << "Supported formats are 'text' and 'tsv'.";

  const bool is_tsv = trainer_spec_.input_format() == "tsv";

  std::set<absl::string_view> meta_pieces_set;
  for (const auto &it : meta_pieces_) meta_pieces_set.insert(it.second.first);
  const PrefixMatcher meta_pieces_matcher(meta_pieces_set);

  for (const auto &filename : trainer_spec_.input()) {
    SPLOG(INFO) << "Loading corpus: " << filename;
    std::string sentence;
    io::InputBuffer input(filename);
    RETURN_IF_ERROR(input.status());
    while (input.ReadLine(&sentence)) {
      int64 freq = 1;
      if (is_tsv) {
        const std::vector<std::string> v = string_util::Split(sentence, "\t");
        CHECK_EQ_OR_RETURN(v.size(), 2)
            << "Input format must be: word <tab> freq. " << sentence;
        sentence = v[0];
        freq = std::atoll(v[1].c_str());
        CHECK_GE_OR_RETURN(freq, 1);
      }

      constexpr int kMaxLines = 2048;
      if (sentence.size() > kMaxLines) {
        SPLOG(INFO) << "Too long lines (>=" << kMaxLines << " bytes). Skipped.";
        continue;
      }
      if (sentence.find(kUNKStr) != std::string::npos) {
        SPLOG(INFO) << "Reserved chars are found. Skipped: " << sentence;
        continue;
      }
      // Escapes meta symbols so that they are not extract as normal pieces.
      sentence = meta_pieces_matcher.GlobalReplace(sentence, " ");

      // Normalizes sentence with Normalizer.
      // whitespaces are replaced with kWSChar.
      const std::string normalized = normalizer.Normalize(sentence);
      if (sentences_.size() % 100000 == 0) {
        SPLOG(INFO) << "Loading: " << normalized
                  << "\tsize=" << sentences_.size();
      }

      CHECK_OR_RETURN(normalized.find(" ") == std::string::npos)
          << "Normalized string must not include spaces";
      if (normalized.empty()) {
        SPLOG(WARNING) << "Empty string found. removed";
        continue;
      }

      sentences_.emplace_back(normalized, freq);

      if (sentences_.size() ==
          static_cast<size_t>(trainer_spec_.input_sentence_size())) {
        goto END;
      }
    }
  }

END:
  SPLOG(INFO) << "Loaded " << sentences_.size() << " sentences";

  // Count character frequencies.
  int64 all_chars_count = 0;
  std::unordered_map<char32, int64> chars_count;
  for (const auto &w : sentences_) {
    for (const char32 c : string_util::UTF8ToUnicodeText(w.first)) {
      if (!string_util::IsValidCodepoint(c)) continue;
      if (c == 0x0000) {
        SPLOG(INFO)
            << "Found null character. The corpus must be encoded in utf-8.";
        continue;
      }
      if (c == 0x0020) {
        // UTF8ToUnicodeText returns a white space if the text
        // contains an interchange-invalid character.
        CHECK_OR_RETURN(w.first.find(" ") == std::string::npos)
            << "space must not be included in normalized string.";
        continue;
      }
      chars_count[c] += w.second;
      all_chars_count += w.second;
    }
  }
  SPLOG(INFO) << "all chars count=" << all_chars_count;

  // Determines required_chars which must be included in the vocabulary.
  int64 accumulated_chars_count = 0;
  for (const auto &w : Sorted(chars_count)) {
    const float coverage = 1.0 * accumulated_chars_count / all_chars_count;
    if (coverage >= trainer_spec_.character_coverage()) {
      SPLOG(INFO) << "Done: " << 100.0 * coverage << "% characters are covered.";
      break;
    }
    accumulated_chars_count += w.second;
    CHECK_NE_OR_RETURN(w.first, 0x0020)
        << "space must not be included in normalized string.";
    required_chars_.insert(w);
  }
  SPLOG(INFO) << "alphabet size=" << required_chars_.size();

  CHECK_OR_RETURN(!port::ContainsKey(required_chars_, kUNKChar));

  // Replaces rare characters (characters not included in required_chars_)
  // with kUNKChar.
  for (auto &w : sentences_) {
    string_util::UnicodeText uw2;
    for (const char32 c : string_util::UTF8ToUnicodeText(w.first)) {
      if (port::ContainsKey(required_chars_, c)) {
        uw2.push_back(c);
      } else {
        uw2.push_back(kUNKChar);
      }
    }
    w.first = string_util::UnicodeTextToUTF8(uw2);
  }

  // +3 for meta pieces.
  if (trainer_spec_.model_type() != TrainerSpec::WORD &&
      trainer_spec_.model_type() != TrainerSpec::CHAR) {
    CHECK_LT_OR_RETURN(
        static_cast<int>(required_chars_.size() + meta_pieces_.size()),
        trainer_spec_.vocab_size())
        << "Vocabulary size is smaller than required_chars. "
        << trainer_spec_.vocab_size() << " vs "
        << required_chars_.size() + meta_pieces_.size() << ". "
        << "Increase vocab_size or decrease character_coverage with "
        << "--character_coverage option.";
  }

  SPLOG(INFO) << "Done! " << sentences_.size() << " sentences are loaded";

  return util::OkStatus();
}

void TrainerInterface::SplitSentencesByWhitespace() {
  SPLOG(INFO) << "Tokenizing input sentences with whitespace: "
            << sentences_.size();
  std::unordered_map<std::string, int64> tokens;
  for (const auto &s : sentences_) {
    for (const auto &w : SplitIntoWords(s.first)) {
      tokens[std::string(w)] += s.second;
    }
  }
  sentences_ = Sorted(tokens);
  SPLOG(INFO) << "Done! " << sentences_.size();
}

util::Status TrainerInterface::Serialize(ModelProto *model_proto) const {
  RETURN_IF_ERROR(status());

  // Duplicated sentencepiece is not allowed.
  std::set<std::string> dup;

#define CHECK_PIECE(piece)                                  \
  CHECK_OR_RETURN(string_util::IsStructurallyValid(piece)); \
  CHECK_OR_RETURN(!piece.empty());                          \
  CHECK_OR_RETURN(dup.insert(piece).second) << piece << " is already defined";

  size_t fid = 0;
  for (int id = 0; id < trainer_spec_.vocab_size(); ++id) {
    const auto it = meta_pieces_.find(id);
    if (it != meta_pieces_.end()) {
      auto *sp = model_proto->add_pieces();
      sp->set_piece(it->second.first);
      sp->set_type(it->second.second);
      sp->set_score(0.0);
      CHECK_EQ_OR_RETURN(model_proto->pieces_size() - 1, it->first);
      CHECK_NE_OR_RETURN(ModelProto::SentencePiece::NORMAL, sp->type());
      CHECK_PIECE(sp->piece());
    } else if (fid < final_pieces_.size()) {
      const auto &w = final_pieces_[fid++];
      auto *sp = model_proto->add_pieces();
      sp->set_piece(w.first);
      sp->set_score(w.second);
      CHECK_PIECE(sp->piece());
    }
  }

  CHECK_EQ_OR_RETURN(fid, final_pieces_.size());

  *(model_proto->mutable_trainer_spec()) = trainer_spec_;
  *(model_proto->mutable_normalizer_spec()) = normalizer_spec_;

  if (!trainer_spec_.hard_vocab_limit() ||
      trainer_spec_.model_type() == TrainerSpec::CHAR) {
    CHECK_GE_OR_RETURN(trainer_spec_.vocab_size(), model_proto->pieces_size());
    CHECK_GE_OR_RETURN(trainer_spec_.vocab_size(),
                       static_cast<int32>(dup.size()));
    model_proto->mutable_trainer_spec()->set_vocab_size(
        model_proto->pieces_size());
  } else {
    CHECK_EQ_OR_RETURN(trainer_spec_.vocab_size(), model_proto->pieces_size());
    CHECK_EQ_OR_RETURN(trainer_spec_.vocab_size(),
                       static_cast<int32>(dup.size()));
  }

  return util::OkStatus();
}

util::Status TrainerInterface::SaveModel(absl::string_view filename) const {
  SPLOG(INFO) << "Saving model: " << filename;
  ModelProto model_proto;
  RETURN_IF_ERROR(Serialize(&model_proto));
  std::ofstream ofs(WPATH(filename.data()), OUTPUT_MODE);
  CHECK_OR_RETURN(ofs) << "\"" << filename.data()
                       << "\": " << util::StrError(errno);
  CHECK_OR_RETURN(model_proto.SerializeToOstream(&ofs));
  return util::OkStatus();
}

util::Status TrainerInterface::SaveVocab(absl::string_view filename) const {
  SPLOG(INFO) << "Saving vocabs: " << filename;
  ModelProto model_proto;
  Serialize(&model_proto);
  io::OutputBuffer output(filename);
  RETURN_IF_ERROR(output.status());

  for (const auto &piece : model_proto.pieces()) {
    std::ostringstream os;
    os << piece.piece() << "\t" << piece.score();
    CHECK_OR_RETURN(output.WriteLine(os.str()));
  }

  return util::OkStatus();
}

util::Status TrainerInterface::Save() const {
  RETURN_IF_ERROR(SaveModel(trainer_spec_.model_prefix() + ".model"));
  RETURN_IF_ERROR(SaveVocab(trainer_spec_.model_prefix() + ".vocab"));
  return util::OkStatus();
}

util::Status TrainerInterface::InitMetaPieces() {
  CHECK_OR_RETURN(meta_pieces_.empty());
  bool has_unk = false;

  auto insert_id = [&has_unk, this](int id, const std::string &w) -> bool {
    if (id < 0) return true;
    if (id >= trainer_spec_.vocab_size() ||
        meta_pieces_.find(id) != meta_pieces_.end() ||
        (has_unk && w == ModelInterface::kUNK()))
      return false;
    if (w == ModelInterface::kUNK()) has_unk = true;
    meta_pieces_[id] = std::make_pair(
        w, w == ModelInterface::kUNK() ? ModelProto::SentencePiece::UNKNOWN
                                       : ModelProto::SentencePiece::CONTROL);
    return true;
  };

  CHECK_OR_RETURN(insert_id(trainer_spec_.unk_id(), ModelInterface::kUNK()));
  CHECK_OR_RETURN(insert_id(trainer_spec_.bos_id(), ModelInterface::kBOS()));
  CHECK_OR_RETURN(insert_id(trainer_spec_.eos_id(), ModelInterface::kEOS()));
  CHECK_OR_RETURN(insert_id(trainer_spec_.pad_id(), ModelInterface::kPAD()));

  CHECK_OR_RETURN(has_unk) << ModelInterface::kUNK() << " must be defined.";

  std::set<std::string> dup;

  int id = 0;
  auto insert_meta_symbol = [&id, &dup, this](
                                const std::string &w,
                                ModelProto::SentencePiece::Type type) -> bool {
    if (!dup.insert(w).second) {
      SPLOG(ERROR) << w << " is already defined.";
      return false;
    }

    if (w == ModelInterface::kUNK()) {
      SPLOG(ERROR) << "<unk> must not be defined with --control_symbols and "
                    "--user_defined_symbols.";
      return false;
    }

    if (w == ModelInterface::kBOS() && trainer_spec_.bos_id() >= 0) {
      meta_pieces_[trainer_spec_.bos_id()].second = type;
    } else if (w == ModelInterface::kEOS() && trainer_spec_.eos_id() >= 0) {
      meta_pieces_[trainer_spec_.eos_id()].second = type;
    } else if (w == ModelInterface::kPAD() && trainer_spec_.pad_id() >= 0) {
      meta_pieces_[trainer_spec_.pad_id()].second = type;
    } else {
      while (meta_pieces_.find(id) != meta_pieces_.end()) ++id;
      meta_pieces_[id] = std::make_pair(w, type);
    }
    return true;
  };

  for (const auto &w : trainer_spec_.control_symbols()) {
    CHECK_OR_RETURN(insert_meta_symbol(w, ModelProto::SentencePiece::CONTROL));
  }

  for (const auto &w : trainer_spec_.user_defined_symbols()) {
    CHECK_OR_RETURN(
        insert_meta_symbol(w, ModelProto::SentencePiece::USER_DEFINED));
  }

  return util::OkStatus();
}

}  // namespace sentencepiece
