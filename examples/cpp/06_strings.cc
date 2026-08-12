#include <iostream>
#include <string>
#include <vector>

#include "absl/strings/ascii.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_split.h"
#include "absl/strings/string_view.h"

namespace {

std::string normalizeHeaderName(absl::string_view name) {
  std::string result(name);
  absl::AsciiStrToLower(&result);
  return result;
}

} // namespace

int main() {
  const absl::string_view csv = "edge, auth,orders";
  std::vector<std::string> clusters;
  for (absl::string_view item : absl::StrSplit(csv, ',')) {
    clusters.emplace_back(absl::StripAsciiWhitespace(item));
  }

  int port = 0;
  if (!absl::SimpleAtoi("10000", &port)) {
    std::cerr << "invalid port\n";
    return 1;
  }

  std::cout << absl::StrCat("listener=0.0.0.0:", port) << '\n';
  std::cout << "clusters=" << absl::StrJoin(clusters, " -> ") << '\n';
  std::cout << "header=" << normalizeHeaderName("X-REQUEST-ID") << '\n';
}
