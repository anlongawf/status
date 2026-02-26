# Server Status Monitor

Hệ thống web quản lý và giám sát thông tin phần cứng máy chủ. Project này giúp theo dõi các thông số hệ thống theo thời gian thực như CPU, RAM, Storage (Disk) và Network.

## Tính năng chính

- **Giám sát CPU**: Hiển thị phần trăm sử dụng CPU, nhiệt độ, tần số hoạt động của các lõi (cores).
- **Giám sát RAM**: Theo dõi dung lượng RAM tổng, đã sử dụng, còn trống và bộ nhớ Swap/Virtual Memory.
- **Giám sát Ổ cứng (Disk)**: Quản lý không gian lưu trữ, phân vùng đĩa, tốc độ đọc/ghi (I/O).
- **Giám sát Mạng (Network)**: Băng thông vào/ra (Inbound/Outbound traffic), độ trễ (latency), số lượng kết nối đang hoạt động.
- **Cảnh báo (Alerting)**: Tự động gửi thông báo khi tài nguyên máy chủ vượt ngưỡng an toàn (ví dụ: RAM > 90%).

## Cài đặt

1. Clone repository về máy:
   ```bash
   git clone git@github.com:anlongawf/status.git
   ```

2. Cài đặt các dependencies cần thiết (nếu có):
   ```bash
   npm install
   # hoặc pip install -r requirements.txt
   ```

3. Khởi chạy hệ thống:
   ```bash
   npm start
   # hoặc python main.py
   ```

## Đóng góp
Nếu bạn muốn đóng góp cho dự án, vui lòng tạo Pull Request hoặc mở Issue mới.
