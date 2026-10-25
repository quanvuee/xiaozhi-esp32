#ifndef __UART_DOG_CONTROLLER_H__
#define __UART_DOG_CONTROLLER_H__

#include <driver/gpio.h>
#include <driver/uart.h>
#include <esp_log.h>
#include <string>
#include "mcp_server.h"

/**
 * @brief Robot Dog Controller via UART & MCP Server
 *
 * Cho phép Xiaozhi Bot gọi các công cụ MCP để điều khiển robot chó (tiến, lùi, dừng).
 * Các lệnh sau đó được phát qua UART TX (mặc định GPIO 4) tới bo mạch điều khiển thực tế.
 */
class UartDogController {
private:
    static constexpr const char* TAG = "UartDogController";

    uart_port_t uart_port_;
    gpio_num_t tx_pin_;

    // Lệnh gửi qua UART tới board điều khiển chuyển động của robot chó
    std::string cmd_forward_ = "FORWARD\n";
    std::string cmd_backward_ = "BACKWARD\n";
    std::string cmd_stop_ = "STOP\n";

    void InitializeUart(int baud_rate) {
        if (tx_pin_ == GPIO_NUM_NC) {
            ESP_LOGW(TAG, "Chân TX UART chưa được cấu hình (GPIO_NUM_NC)");
            return;
        }

        uart_config_t uart_config = {
            .baud_rate = baud_rate,
            .data_bits = UART_DATA_8_BITS,
            .parity = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
            .rx_flow_ctrl_thresh = 0,
            .source_clk = UART_SCLK_DEFAULT,
        };

        if (!uart_is_driver_installed(uart_port_)) {
            ESP_ERROR_CHECK(uart_driver_install(uart_port_, 256, 256, 0, NULL, 0));
        }

        ESP_ERROR_CHECK(uart_param_config(uart_port_, &uart_config));
        ESP_ERROR_CHECK(uart_set_pin(uart_port_, tx_pin_, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE,
                                     UART_PIN_NO_CHANGE));

        ESP_LOGI(TAG, "Đã khởi tạo UART cổng %d, TX tại GPIO %d, baud rate: %d", uart_port_,
                 tx_pin_, baud_rate);
    }

    void RegisterMcpTools() {
        auto& mcp_server = McpServer::GetInstance();

        // 1. Tool tiến về phía trước (Move Forward)
        mcp_server.AddTool("self.dog.move_forward",
                           "Điều khiển robot chó di chuyển tiến về phía trước.", PropertyList(),
                           [this](const PropertyList&) -> ReturnValue {
                               SendUartCommand(cmd_forward_);
                               return std::string("Robot chó đang tiến về phía trước.");
                           });

        // Bí danh forward
        mcp_server.AddTool("self.dog.forward", "Điều khiển robot chó tiến lên phía trước.",
                           PropertyList(), [this](const PropertyList&) -> ReturnValue {
                               SendUartCommand(cmd_forward_);
                               return std::string("Robot chó đang tiến về phía trước.");
                           });

        // 2. Tool lùi lại phía sau (Move Backward)
        mcp_server.AddTool("self.dog.move_backward",
                           "Điều khiển robot chó di chuyển lùi về phía sau.", PropertyList(),
                           [this](const PropertyList&) -> ReturnValue {
                               SendUartCommand(cmd_backward_);
                               return std::string("Robot chó đang lùi lại phía sau.");
                           });

        // Bí danh backward
        mcp_server.AddTool("self.dog.backward", "Điều khiển robot chó lùi lại phía sau.",
                           PropertyList(), [this](const PropertyList&) -> ReturnValue {
                               SendUartCommand(cmd_backward_);
                               return std::string("Robot chó đang lùi lại phía sau.");
                           });

        // 3. Tool dừng lại (Stop)
        mcp_server.AddTool("self.dog.stop",
                           "Dừng mọi di chuyển hoặc hoạt động của robot chó ngay lập tức.",
                           PropertyList(), [this](const PropertyList&) -> ReturnValue {
                               SendUartCommand(cmd_stop_);
                               return std::string("Robot chó đã dừng lại.");
                           });

        // 4. Tool điều khiển tổng hợp với tham số action
        mcp_server.AddTool(
            "self.dog.control",
            "Điều khiển hướng di chuyển của robot chó. Tham số action có thể nhận một trong các "
            "giá trị: "
            "'forward' (tiến lên), 'backward' (lùi lại), 'stop' (dừng lại).",
            PropertyList({Property("action", kPropertyTypeString)}),
            [this](const PropertyList& properties) -> ReturnValue {
                const std::string& action = properties["action"].value<std::string>();
                if (action == "forward") {
                    SendUartCommand(cmd_forward_);
                    return std::string("Robot chó đang tiến về phía trước.");
                } else if (action == "backward") {
                    SendUartCommand(cmd_backward_);
                    return std::string("Robot chó đang lùi lại phía sau.");
                } else if (action == "stop") {
                    SendUartCommand(cmd_stop_);
                    return std::string("Robot chó đã dừng lại.");
                } else {
                    return std::string(
                        "Lỗi: Lệnh không hợp lệ. Vui lòng chọn 'forward', 'backward' hoặc 'stop'.");
                }
            });

        ESP_LOGI(TAG,
                 "Đã đăng ký thành công các công cụ MCP điều khiển robot chó bằng tiếng Việt.");
    }

public:
    /**
     * @param tx_pin Chân TX kết nối tới bo mạch điều khiển chó (Mặc định GPIO 4)
     * @param uart_port Cổng UART ESP32 sử dụng (Mặc định UART_NUM_1)
     * @param baud_rate Tốc độ baud giao tiếp UART (Mặc định 115200)
     */
    UartDogController(gpio_num_t tx_pin = GPIO_NUM_4, uart_port_t uart_port = UART_NUM_1,
                      int baud_rate = 115200)
        : uart_port_(uart_port), tx_pin_(tx_pin) {
        InitializeUart(baud_rate);
        RegisterMcpTools();
    }

    /**
     * @brief Tùy chỉnh nội dung lệnh gửi qua UART nếu board nhận yêu cầu giao thức khác
     */
    void SetCommands(std::string forward, std::string backward, std::string stop) {
        cmd_forward_ = std::move(forward);
        cmd_backward_ = std::move(backward);
        cmd_stop_ = std::move(stop);
    }

    /**
     * @brief Gửi chuỗi lệnh bất kỳ qua UART TX
     */
    bool SendUartCommand(const std::string& command) {
        if (command.empty()) {
            return false;
        }
        int bytes_written = uart_write_bytes(uart_port_, command.data(), command.size());
        if (bytes_written < 0) {
            ESP_LOGE(TAG, "Gửi dữ liệu qua UART thất bại");
            return false;
        }
        ESP_LOGI(TAG, "Đã gửi lệnh UART [%d bytes]: %s", bytes_written, command.c_str());
        return true;
    }
};

#endif  // __UART_DOG_CONTROLLER_H__
