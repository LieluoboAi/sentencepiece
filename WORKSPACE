
workspace(name = "github_sentencepiece")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")


skylib_version = "0.8.0"

http_archive(
    name = "bazel_skylib",
    sha256 = "2ef429f5d7ce7111263289644d233707dba35e39696377ebab8b0bc701f7818e",
    type = "tar.gz",
    url = "https://github.com/bazelbuild/bazel-skylib/releases/download/{}/bazel-skylib.{}.tar.gz".format(skylib_version, skylib_version),
)


http_archive(
    name = "com_google_protobuf",
    sha256 = "f1748989842b46fa208b2a6e4e2785133cfcc3e4d43c17fecb023733f0f5443f",
    strip_prefix = "protobuf-3.7.1",
    urls = [
        "https://github.com/protocolbuffers/protobuf/archive/v3.7.1.tar.gz",
    ],
)


http_archive(
    name = "com_google_protobuf_cc",
    sha256 = "f1748989842b46fa208b2a6e4e2785133cfcc3e4d43c17fecb023733f0f5443f",
    strip_prefix = "protobuf-3.7.1",
    urls = [
        "https://github.com/protocolbuffers/protobuf/archive/v3.7.1.tar.gz",
    ],
)


bind(
    name = "protocol_compiler",
    actual = "@com_google_protobuf//:protoc",
)

bind(
    name = "protobuf_headers",
    actual = "@com_google_protobuf//:protobuf_headers",
)

http_archive(
    name = "zlib_archive",
    build_file = "//:zlib.BUILD",
    sha256 = "36658cb768a54c1d4dec43c3116c27ed893e88b02ecfcb44f2166f9c0b7f2a0d",
    strip_prefix = "zlib-1.2.8",
    urls = [
        # "http://bazel-mirror.storage.googleapis.com/zlib.net/zlib-1.2.8.tar.gz",
        "http://zlib.net/fossils/zlib-1.2.8.tar.gz",
    ],
)


http_archive(
    name = "com_google_absl",
    build_file = "//:absl.BUILD",
    sha256 = "7dd09690ae7ca4551de3111d4a86b75b23ec17445f273d3c42bdcdc1c7b02e4e",
    strip_prefix = "abseil-cpp-48cd2c3f351ff188bc85684b84a91b6e6d17d896",
    urls = [
        "https://mirror.bazel.build/github.com/abseil/abseil-cpp/archive/48cd2c3f351ff188bc85684b84a91b6e6d17d896.tar.gz",
        "https://github.com/abseil/abseil-cpp/archive/48cd2c3f351ff188bc85684b84a91b6e6d17d896.tar.gz",
    ],
)

bind(
    name = "zlib",
    actual = "@zlib_archive//:zlib",
)
