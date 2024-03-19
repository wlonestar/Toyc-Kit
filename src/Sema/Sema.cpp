#include <Sema/Sema.h>

namespace toyc {

bool Sema::checkHexadecimal(std::string value) {
  if (value.starts_with("0x") || value.starts_with("0X")) {
    return true;
  }
  return false;
}

bool Sema::checkOctal(std::string value) {
  if (value.starts_with("0") && value.size() != 1) {
    return true;
  }
  return false;
}

std::string Sema::checkUnaryOperatorType(ExprPtr &rhs, TokenTy type) {
  if (type == NOT) {
    return "i64";
  }
  return rhs->getType();
}

std::string Sema::checkBinaryOperatorType(ExprPtr &lhs, ExprPtr &rhs,
                                          TokenTy type) {
  if (type == AND_OP || type == OR_OP) {
    return "i64";
  }
  if (lhs->getType() == rhs->getType()) {
    lhs = std::make_unique<ImplicitCastExpr>(lhs->getType(), std::move(lhs));
    rhs = std::make_unique<ImplicitCastExpr>(rhs->getType(), std::move(rhs));
    return lhs->getType();
  }
  if (lhs->getType() == "f64" || rhs->getType() == "f64") {
    lhs = std::make_unique<ImplicitCastExpr>("f64", std::move(lhs));
    rhs = std::make_unique<ImplicitCastExpr>("f64", std::move(rhs));
    return "f64";
  }
  return "i64";
}

std::string Sema::checkShiftOperatorType(ExprPtr &lhs, ExprPtr &rhs,
                                         TokenTy type) {
  if (lhs->getType() == "f64" || rhs->getType() == "f64") {
    return "";
  }
  return "i64";
}

} // namespace toyc
