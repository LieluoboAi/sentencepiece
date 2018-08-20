package(default_visibility = ["//visibility:public"])

proto_library(
    name = "sentencepiece_model_proto",
    srcs = [
        "sentencepiece_model.proto",
    ],
)

cc_proto_library(
    name = "sentencepiece_model_cc_proto",
    deps = [":sentencepiece_model_proto"],
)

proto_library(
    name = "sentencepiece_proto",
    srcs = [
        "sentencepiece.proto",
    ],
)

cc_proto_library(
    name = "sentencepiece_cc_proto",
    deps = [":sentencepiece_proto"],
)


cc_library(
    name = "common",
    hdrs = ["common.h"],
)

# https://docs.bazel.build/versions/master/be/c-cpp.html#cc_library
cc_library(
    name = "sentencepiece_processor",
    srcs = ["sentencepiece_processor.cc"],
    hdrs = ["sentencepiece_processor.h"],
    deps = [
        ":common",
        "@com_google_absl//absl/strings",
        ":model_factory",
        ":sentencepiece_cc_proto",
        ":flags",
        ":normalizer",
    ],
)

cc_library(
    name = "util",
    srcs = [
        "util.cc",
        "util.h",
    ],
    deps = [
        ":common",
        "@com_google_absl//absl/strings",
    ],
)

cc_library(
    name = "error",
    srcs = [
        "error.cc",
    ],
    deps = [
        ":util",
    ],
)

cc_library(
    name = "flags",
    srcs = [
        "flags.cc",
        "flags.h",
    ],
    deps = [
        ":util",
    ],
)

cc_library(
    name = "model_interface",
    srcs = [
        "model_interface.cc",
        "model_interface.h",
    ],
    deps = [
        ":sentencepiece_model_cc_proto",
        ":util",
        "//third_party/darts_clone:darts",
    ],
)

cc_library(
    name = "bpe_model",
    srcs = [
        "bpe_model.cc",
        "bpe_model.h",
    ],
    deps = [
        ":model_interface",
    ],
)

cc_library(
    name = "char_model",
    srcs = [
        "char_model.cc",
        "char_model.h",
    ],
    deps = [
        ":model_interface",
    ],
)

cc_library(
    name = "unigram_model",
    srcs = [
        "unigram_model.cc",
        "unigram_model.h",
    ],
    deps = [
        ":model_interface",
    ],
)

cc_library(
    name = "word_model",
    srcs = [
        "word_model.cc",
        "word_model.h",
    ],
    deps = [
        ":model_interface",
    ],
)

# https://docs.bazel.build/versions/master/be/c-cpp.html#cc_library
cc_library(
    name = "model_factory",
    srcs = [
        "model_factory.cc",
        "model_factory.h",
    ],
    deps = [
        ":bpe_model",
        ":char_model",
        ":unigram_model",
        ":word_model",
    ],
)

cc_library(
    name = "normalizer",
    srcs = [
        "normalizer.cc",
        "normalizer.h",
    ],
    deps = [
        ":sentencepiece_model_cc_proto",
        ":util",
        "//third_party/darts_clone:darts",
    ],
)


cc_library(
    name = "builder",
    srcs = [
        "builder.cc",
        "builder.h",
        "normalization_rule.h",
    ],
    deps = [
        ":sentencepiece_model_cc_proto",
        ":util",
        ":normalizer",
        "//third_party/darts_clone:darts",
    ],
)

cc_library(
    name = "unicode_script",
    srcs = [
        "unicode_script.cc",
        "unicode_script.h",
        "unicode_script_map.h",
    ],
    deps = [
        ":util",
    ],
)

cc_library(
    name = "trainer_interface",
    srcs = [
        "trainer_interface.cc",
        "trainer_interface.h",
    ],
    deps = [
        ":util",
        ":unicode_script",
        ":model_factory",
        ":normalizer",
    ],
)


cc_library(
    name = "bpe_model_trainer",
    srcs = [
        "bpe_model_trainer.cc",
        "bpe_model_trainer.h",
    ],
    deps = [
        ":trainer_interface",
    ],
)

cc_library(
    name = "char_model_trainer",
    srcs = [
        "char_model_trainer.cc",
        "char_model_trainer.h",
    ],
    deps = [
        ":trainer_interface",
    ],
)

cc_library(
    name = "unigram_model_trainer",
    srcs = [
        "unigram_model_trainer.cc",
        "unigram_model_trainer.h",
    ],
    deps = [
        ":trainer_interface",
        "//third_party/esaxx:esaxx",
    ],
)

cc_library(
    name = "word_model_trainer",
    srcs = [
        "word_model_trainer.cc",
        "word_model_trainer.h",
    ],
    deps = [
        ":trainer_interface",
    ],
)


cc_library(
    name = "trainer_factory",
    srcs = [
        "trainer_factory.cc",
        "trainer_factory.h",
    ],
    deps = [
        ":bpe_model_trainer",
        ":char_model_trainer",
        ":unigram_model_trainer",
        ":word_model_trainer",
    ],
)

cc_library(
    name = "sentencepiece_trainer",
    srcs = [
        "sentencepiece_trainer.cc",
        "sentencepiece_trainer.h",
    ],
    deps = [
        ":trainer_factory",
        ":normalizer",
        ":builder",
        ":flags",
        ":sentencepiece_cc_proto",
    ],
)

cc_binary(
    name = "spm_train",
    srcs = [
        "spm_train_main.cc",
    ],
    deps = [
        ":sentencepiece_trainer",
        ":error",
    ],
)

cc_binary(
    name = "spm_decode",
    srcs = [
        "spm_decode_main.cc",
    ],
    deps = [
        ":model_factory",
        ":sentencepiece_processor",
        ":error",
    ],
)

cc_binary(
    name = "spm_encode",
    srcs = [
        "spm_encode_main.cc",
    ],
    deps = [
        ":model_factory",
        ":sentencepiece_trainer",
        ":sentencepiece_processor",
        ":error",
    ],
)


cc_binary(
    name = "spm_normalize",
    srcs = [
        "spm_normalize_main.cc",
    ],
    deps = [
        ":model_factory",
        ":sentencepiece_trainer",
        ":sentencepiece_processor",
        ":error",
    ],
)
