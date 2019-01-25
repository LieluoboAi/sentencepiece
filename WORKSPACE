
workspace(name = "github_sentencepiece")

http_archive(
    name = "io_bazel_rules_closure",
    sha256 = "a38539c5b5c358548e75b44141b4ab637bba7c4dc02b46b1f62a96d6433f56ae",
    strip_prefix = "rules_closure-dbb96841cc0a5fb2664c37822803b06dab20c7d1",
    urls = [
        "https://mirror.bazel.build/github.com/bazelbuild/rules_closure/archive/dbb96841cc0a5fb2664c37822803b06dab20c7d1.tar.gz",
        "https://github.com/bazelbuild/rules_closure/archive/dbb96841cc0a5fb2664c37822803b06dab20c7d1.tar.gz",  # 2018-04-13
    ],
)


new_http_archive(
    name = "com_google_absl",
    sha256 = "7dd09690ae7ca4551de3111d4a86b75b23ec17445f273d3c42bdcdc1c7b02e4e",
    strip_prefix = "abseil-cpp-48cd2c3f351ff188bc85684b84a91b6e6d17d896",
    urls = [
        "https://mirror.bazel.build/github.com/abseil/abseil-cpp/archive/48cd2c3f351ff188bc85684b84a91b6e6d17d896.tar.gz",
        "https://github.com/abseil/abseil-cpp/archive/48cd2c3f351ff188bc85684b84a91b6e6d17d896.tar.gz",
    ],
    build_file="third_party/com_google_absl.BUILD"
)


load("@io_bazel_rules_closure//closure:defs.bzl", "closure_repositories")

closure_repositories()