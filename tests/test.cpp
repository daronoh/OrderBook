#include "pch.h"

#include "../src/OrderBook.cpp"
#include <iostream>

namespace googletest = ::testing;

enum class ActionType {
  Add,
  Modify,
  Cancel,
};

struct Information {
  ActionType type_;
  OrderType orderType_;
  Side side_;
  Price price_;
  Quantity quantity_;
  OrderId orderId_;
};

using Informations = std::vector<Information>;

// improvement: specify a vector of orders and compare
struct Result {
  std::size_t allCount_;
  std::size_t bidCount_;
  std::size_t askCount_;
};

struct InputHandler {
private:
  std::uint64_t ToNumber(const std::string_view &str) const {
    std::int64_t value{};
    std::from_chars(str.data(), str.data() + str.size(), value);
    if (value < 0)
      throw std::logic_error("Value is below zero.");
    return static_cast<std::uint64_t>(value);
  }

  bool TryParseResult(const std::string_view &str, Result &result) const {
    if (str.at(0) != 'R')
      return false;

    auto values = Split(str, ' ');
    result.allCount_ = ToNumber(values.at(1));
    result.bidCount_ = ToNumber(values.at(2));
    result.askCount_ = ToNumber(values.at(3));

    return true;
  }

  bool TryParseInformation(const std::string_view &str,
                           Information &info) const {
    auto action = str.at(0);
    auto values = Split(str, ' ');
    if (action == 'A') {
      info.type_ = ActionType::Add;
      info.orderType_ = ParseOrderType(values.at(1));
      info.orderId_ = ParseOrderId(values.at(2));
      info.side_ = ParseSide(values.at(3));
      info.price_ = ParsePrice(values.at(4));
      info.quantity_ = ParseQuantity(values.at(5));
    } else if (action == 'M') {
      info.type_ = ActionType::Modify;
      info.orderId_ = ParseOrderId(values.at(1));
      info.price_ = ParsePrice(values.at(2));
      info.quantity_ = ParseQuantity(values.at(3));
    } else if (action == 'C') {
      info.type_ = ActionType::Cancel;
      info.orderId_ = ParseOrderId(values.at(1));
    } else
      return false;

    return true;
  }

  std::vector<std::string_view> Split(const std::string_view &str,
                                      char delimiter) const {
    std::vector<std::string_view> columns{};
    std::size_t startIndex{}, endIndex{};
    while ((endIndex = str.find(delimiter, startIndex)) &&
           endIndex != std::string::npos) {
      auto distance = endIndex - startIndex;
      auto column = str.substr(startIndex, distance);
      columns.push_back(column);
      startIndex = endIndex + 1;
    }

    columns.push_back(str.substr(startIndex));
    return columns;
  }

  OrderType ParseOrderType(const std::string_view &str) const {
    if (str == "FAK")
      return OrderType::FillAndKill;
    else if (str == "FOK")
      return OrderType::FillOrKill;
    else if (str == "GFD")
      return OrderType::GoodForDay;
    else if (str == "GTC")
      return OrderType::GoodTillCancel;
    else if (str == "M")
      return OrderType::Market;
    else
      throw std::logic_error(std::format("Unknown Order Type: {}", str));
  }
  OrderId ParseOrderId(const std::string_view &str) const {
    if (str.empty())
      throw std::logic_error("Order Id empty!");

    return ToNumber(str);
  }
  Side ParseSide(const std::string_view &str) const {
    if (str == "B")
      return Side::Buy;
    else if (str == "S")
      return Side::Sell;
    else
      throw std::logic_error(std::format("Unknown Side: {}", str));
  }
  Price ParsePrice(const std::string_view &str) const {
    if (str.empty())
      throw std::logic_error("Price empty!");

    return ToNumber(str);
  }
  Quantity ParseQuantity(const std::string_view &str) const {
    if (str.empty())
      throw std::logic_error("Quantity empty!");

    return ToNumber(str);
  }

public:
  std::tuple<Informations, Result>
  GetInformations(const std::filesystem::path &path) const {
    Informations infos;
    infos.reserve(1'000);

    std::string line;
    std::ifstream file{path};
    while (std::getline(file, line)) {
      if (line.empty())
        break;

      const bool isResult = line.at(0) == 'R';
      const bool isUpdate = !isResult;

      if (isUpdate) {
        Information update;

        auto isValid = TryParseInformation(line, update);
        if (!isValid)
          throw std::logic_error(
              std::format("({}) update command is invalid", line));

        infos.push_back(update);
      } else {
        if (!file.eof())
          throw std::logic_error("Result must be at the end of the file only.");

        Result result;

        auto isValid = TryParseResult(line, result);
        if (!isValid)
          throw std::logic_error(std::format("({}) command is invalid", line));

        return {infos, result};
      }
    }

    throw std::logic_error("No result specified.");
  }
};

class OrderbookTestsFixture : public googletest::TestWithParam<const char *> {
public:
  const static inline std::filesystem::path TestFolderPath{
      std::filesystem::path{SOURCE_DIR} / "tests" / "TestFiles"};
};

TEST_P(OrderbookTestsFixture, OrderbookTestSuite) {
  // Arrange
  const auto file = OrderbookTestsFixture::TestFolderPath / GetParam();

  InputHandler handler;
  const auto [updates, result] = handler.GetInformations(file);

  auto GetOrder = [](const Information &information) {
    return std::make_shared<Order>(information.orderType_, information.orderId_,
                                   information.side_, information.price_,
                                   information.quantity_);
  };

  auto GetOrderModify = [](const Information &information) {
    return OrderModify{information.orderId_, information.price_,
                       information.quantity_};
  };

  // Act
  Orderbook orderbook;
  for (const auto &update : updates) {
    switch (update.type_) {
    case ActionType::Add: {
      const Trades &trades = orderbook.AddOrder(GetOrder(update));
    } break;
    case ActionType::Modify: {
      const Trades &trades = orderbook.ModifyOrder(GetOrderModify(update));
    } break;
    case ActionType::Cancel: {
      orderbook.CancelOrder(update.orderId_);
    } break;
    default:
      throw std::logic_error("Unsupported update");
    }
  }

  // Assert
  const auto &orderbookInfos = orderbook.GetOrderInfos();
  ASSERT_EQ(orderbook.Size(), result.allCount_);
  ASSERT_EQ(orderbookInfos.GetBids().size(), result.bidCount_);
  ASSERT_EQ(orderbookInfos.GetAsks().size(), result.askCount_);
}

INSTANTIATE_TEST_CASE_P(
    Tests, OrderbookTestsFixture,
    googletest::ValuesIn({"Match_GoodTillCancel.txt", "Match_FillAndKill.txt",
                          "Match_FillOrKill_Hit.txt",
                          "Match_FillOrKill_Miss.txt", "Cancel_Success.txt",
                          "Modify_Price.txt", "Match_Market.txt"}));