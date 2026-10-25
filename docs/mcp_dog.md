  ### 1. Chi tiết triển khai

  1. Module điều khiển Robot Chó qua UART & MCP:
      • Tạo file uart_dog_controller.h trong thư mục dùng
      chung main/boards/common/ (theo chuẩn thiết kế tương
      tự LampController).
      • Cấu hình UART port (UART_NUM_1), tốc độ truyền
      115200 baud, và kích hoạt chân phát UART TX tại GPIO 4
      (GPIO_NUM_4).
      • Gửi lệnh văn bản qua UART xuống bo mạch chấp hành:
          • Tiến về phía trước: "FORWARD\n"
          • Lùi lại phía sau: "BACKWARD\n"
          • Dừng lại: "STOP\n"
          (Có thể tùy biến chuỗi lệnh qua hàm SetCommands
          nếu bo mạch nhận cần định dạng khác).

  2. Các công cụ MCP đăng ký với mô tả bằng tiếng Việt:
      • self.dog.move_forward (hoặc bí danh self.dog.
      forward):
          • Mô tả: "Điều khiển robot chó di chuyển tiến về
          phía trước."
          • Phản hồi: "Robot chó đang tiến về phía trước."
      • self.dog.move_backward (hoặc bí danh self.dog.
      backward):
          • Mô tả: "Điều khiển robot chó di chuyển lùi về
          phía sau."
          • Phản hồi: "Robot chó đang lùi lại phía sau."
      • self.dog.stop:
          • Mô tả: "Dừng mọi di chuyển hoặc hoạt động của
          robot chó ngay lập tức."
          • Phản hồi: "Robot chó đã dừng lại."
      • self.dog.control (công cụ tổng hợp nhận tham số
      action):
          • Mô tả: "Điều khiển hướng di chuyển của robot chó.
          Tham số action có thể nhận một trong các giá trị:
          'forward' (tiến lên), 'backward' (lùi lại), 'stop'
          (dừng lại)."

  3. Tích hợp vào Bo mạch:
      • Cập nhật trong esp-sensairshuttle.cc:284-295:
        #include "uart_dog_controller.h"
        
        void InitializeTools() {
            static UartDogController
            ￹dog_controller(GPIO_NUM_4);
        }
      và gọi InitializeTools(); trong hàm khởi tạo bo mạch.

  ──────
  ### 2. Sơ đồ kết nối phần cứng

    +---------------------------+
    ￹               +--------------------------+
    |  Xiaozhi ESP32 Bot Board  |                |   Motor /
    ￹Dog Controller |
    |                           |                |
    ￹      (Board nhận)       |
    |            GPIO 4 (UART TX) -------------> | RX
    ￹                      |
    |                       GND ---------------- | GND
    ￹                     |
    +---------------------------+
    ￹               +--------------------------+
  ──────
  ### 3. Kết quả xác minh (Build Validation)

  • Mã nguồn đã được định dạng chuẩn với clang-format.
  • Quá trình biên dịch và liên kết firmware đã hoàn tất
  thành công:
      • Binary sinh ra: build/xiaozhi.bin (kích thước
      0x2333b0 bytes, còn trống 44% phân vùng app).
