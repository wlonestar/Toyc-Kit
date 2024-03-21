#include <Expected.h>
#include <Util.h>

#include <gtest/gtest.h>
#include <memory>
#include <ostream>
#include <sstream>

namespace toyc {

#define errorMsg(a, b)

class ExpectedTest : public testing::Test {};

enum CalculateErrorCode {
  DivisionZero
};

Expected<int> divide(int a, int b) {
  if (b == 0) {
    return makeError(DivisionZero, makeString("zero: '{}'/'{}'", a, b));
  }
  return a / b;
}

TEST_F(ExpectedTest, SimpleTest) {
  std::vector<std::pair<int, int>> array{
      {1, 1}, {2, 0}, {8, 3}, {9, 4}, {10, 2}};
  for (auto [a, b] : array) {
    if (auto result = divide(a, b)) {
      EXPECT_EQ(result.getValue(), a / b);
    } else {
      EXPECT_EQ(result.getError().message(), fstr("zero: '{}'/'{}'", a, b));
    }
  }
}

class Node {
public:
  int value;
  std::unique_ptr<Node> next;

  Node(int _val, std::unique_ptr<Node> _next = nullptr)
      : value(_val), next(std::move(_next)) {}
};

void printList(std::unique_ptr<Node> &head, std::ostream &os) {
  while (head->next != nullptr) {
    os << head->value << " ";
    head = std::move(head->next);
  }
  os << "\n";
}

enum NodeErrorCode { GayNumber };

Expected<std::unique_ptr<Node>> buildTree(std::vector<int> &array) {
  auto tail = std::make_unique<Node>(-1);
  for (auto &item : array) {
    if (item == 114514) {
      return makeError(GayNumber, "illegal number");
    }
    auto node = std::make_unique<Node>(item, std::move(tail));
    tail = std::move(node);
  }
  return tail;
}

TEST_F(ExpectedTest, PtrTest) {
  std::vector<int> v{5, 4, 3, 2, 114514};
  if (auto result = buildTree(v)) {
    auto list = result.getValue();
    std::ostringstream os;
    printList(list, os);
    EXPECT_EQ(os.str(), "1 2 3 4 5 \n");
  } else {
    auto err = result.getError();
    EXPECT_EQ(err.errorCode(), GayNumber);
    EXPECT_EQ(err.message(), "illegal number");
  }
}

} // namespace toyc
