#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

class Listener : public rclcpp::Node {
public:
    Listener() : Node("listener") {
        sub_ = this->create_subscription<std_msgs::msg::String>(
            "/chat", 10,
            std::bind(&Listener::callback, this, std::placeholders::_1)
        );
    }

private:
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr sub_;
    void callback(const std_msgs::msg::String::SharedPtr msg) {
        RCLCPP_INFO(this->get_logger(), "Received: %s", msg->data.c_str());
    }
};

int main(int argc, char * argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<Listener>());
    rclcpp::shutdown();
    return 0;
}

