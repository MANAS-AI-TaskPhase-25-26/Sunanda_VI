#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include <iostream>

class Talker : public rclcpp::Node {
public:
    Talker() : Node("talker") {
        pub_ = this->create_publisher<std_msgs::msg::String>("/chat", 10);
        std::cout <<"Enter your name";
        std::getline(std::cin, name);
        // Timer for continuous sending
        timer_ = this->create_wall_timer(
            std::chrono::milliseconds(500),
            std::bind(&Talker::send_message, this)
        );
    }
private:
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr pub_;
    rclcpp::TimerBase::SharedPtr timer_;
    //.. 
    std::string name;   
    void send_message() {
        std_msgs::msg::String msg; 
        std::string input;
        std::cout << "you";
        std::getline(std::cin, input);
        msg.data = name+":"+input;
        pub_->publish(msg);
        RCLCPP_INFO(this->get_logger(), "Sent: %s", msg.data.c_str());
         
    }
           
};

int main(int argc, char * argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<Talker>());
    rclcpp::shutdown();
    return 0;
}
