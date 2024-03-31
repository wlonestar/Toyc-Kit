#include <Sema/Sema.h>

namespace toyc {

auto Sema::CheckHexadecimal(const std::string &value) -> bool {
  return value.starts_with("0x") || value.starts_with("0X");
}

auto Sema::CheckOctal(const std::string &value) -> bool {
  return value.starts_with("0") && value.size() != 1;
}

auto Sema::CheckUnaryOperatorType(ExprPtr &rhs, TokenTy type) -> std::string {
  if (type == NOT) {
    return "i64";
  }
  return rhs->GetType();
}

auto Sema::CheckBinaryOperatorType(ExprPtr &lhs, ExprPtr &rhs, TokenTy type)
    -> std::string {
  if (type == AND_OP || type == OR_OP) {
    return "i64";
  }
  if (lhs->GetType() == rhs->GetType()) {
    lhs = std::make_unique<ImplicitCastExpr>(lhs->GetType(), std::move(lhs));
    rhs = std::make_unique<ImplicitCastExpr>(rhs->GetType(), std::move(rhs));
    return lhs->GetType();
  }
  if (lhs->GetType() == "f64" || rhs->GetType() == "f64") {
    lhs = std::make_unique<ImplicitCastExpr>("f64", std::move(lhs));
    rhs = std::make_unique<ImplicitCastExpr>("f64", std::move(rhs));
    return "f64";
  }
  return "i64";
}

auto Sema::CheckShiftOperatorType(ExprPtr &lhs, ExprPtr &rhs, TokenTy type)
    -> std::string {
  if (lhs->GetType() == "f64" || rhs->GetType() == "f64") {
    return "";
  }
  return "i64";
}

} // namespace toyc
