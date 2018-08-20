
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
        urls = [
            "https://mirror.bazel.build/github.com/abseil/abseil-cpp/archive/9613678332c976568272c8f4a78631a29159271d.tar.gz",
            "https://github.com/abseil/abseil-cpp/archive/9613678332c976568272c8f4a78631a29159271d.tar.gz",
        ],
        sha256 = "1273a1434ced93bc3e703a48c5dced058c95e995c8c009e9bdcb24a69e2180e9",
        strip_prefix = "abseil-cpp-9613678332c976568272c8f4a78631a29159271d",
        build_file = "//third_party:com_google_absl.BUILD",
)


load("@io_bazel_rules_closure//closure:defs.bzl", "closure_repositories")

closure_repositories()