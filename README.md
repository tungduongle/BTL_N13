# BTL_N13 — Hệ thống giám sát và cảnh báo rò rỉ khí gas

## 1. Tổng quan hệ thống

Dự án sử dụng **STM32F103C8T6**, cảm biến **MQ-2** và **FreeRTOS** để đọc tín hiệu khí gas, ước lượng nồng độ LPG/propane, hiển thị trên OLED SSD1306 và điều khiển còi cảnh báo. Hai nút nhấn cho phép thử còi, tạm tắt âm thanh và điều chỉnh ngưỡng cảnh báo.

Luồng xử lý chính:

```text
MQ-2 → ADC → Task Sensor → Queue dữ liệu → Task Alarm → Còi
                                             │
Nút ALARM / SET → Task Button → Notification ──┤
                                             ↓
                                      Queue hiển thị
                                             ↓
                                         Task OLED
                                             ↓
                                     OLED + đồ thị lịch sử
```

Các tác vụ được phân chia như sau:

| Tác vụ | Chức năng | Chu kỳ / cơ chế | Ưu tiên so với Idle |
|---|---|---|---|
| Sensor | Lấy 8 mẫu ADC, lọc dữ liệu, quy đổi sang PPM và lưu lịch sử | 100 ms | +3 |
| Alarm | Phân loại trạng thái, xử lý nút, điều khiển còi và lưu ngưỡng | 20 ms | +4 |
| Button | Chống dội, phân biệt nhấn ngắn, nhấn giữ và giữ đồng thời hai nút | Sự kiện nút; chống dội 30 ms | +3 |
| OLED | Hiển thị PPM, ADC, trạng thái, ngưỡng, đếm ngược tắt còi và đồ thị 64 mẫu | 200 ms | +2 |

Queue truyền dữ liệu mới nhất giữa các tác vụ; semaphore phục vụ xử lý nút; mutex bảo vệ lịch sử; task notification truyền sự kiện đến Alarm. Software timer hỗ trợ thời gian tắt còi và thử còi.

### Trạng thái hoạt động

| Trạng thái | Điều kiện chính | Còi |
|---|---|---|
| `SAFE` | Dữ liệu hợp lệ, dưới ngưỡng cảnh báo khi xét trạng thái trước đó | Tắt, trừ lúc thử còi |
| `WARNING` | PPM đạt ngưỡng cảnh báo | Bật 500 ms, tắt 500 ms |
| `DANGER` | PPM đạt ngưỡng nguy hiểm | Bật 100 ms, tắt 100 ms |
| `SENSOR FAIL` | Không có dữ liệu hợp lệ quá 2 giây | Bật liên tục |
| `ADC LIMIT` | Điện áp tính theo mô hình đạt/vượt điện áp mạch cảm biến | Bật liên tục; hiển thị `PPM: OVER` |

Ngưỡng mặc định hiện tại là **WARNING = 50 PPM**, **DANGER = 80 PPM**. Hysteresis bằng **5 PPM** giúp tránh đổi trạng thái liên tục: để thoát một mức cảnh báo, giá trị phải giảm xuống dưới ngưỡng tương ứng trừ 5.

**Lưu ý về số đo:** các hệ số MQ-2 trong `app/inc/app_config.h` đang là giá trị mẫu chưa hiệu chuẩn. Ngưỡng 50/80 phục vụ demo với cấu hình hiển thị hiện tại, không phải ngưỡng an toàn đã được xác nhận. Với cấu hình này, ADC chạm 4095 vẫn được quy đổi thành khoảng 91 PPM; không thể dùng số đó để đo phần tín hiệu vượt dải ADC.

## 2. Cách demo

### 2.1. Chuẩn bị và nối dây

Chuẩn bị board STM32F103C8T6, MQ-2, OLED SSD1306 I2C 128×64, mạch còi kích mức thấp, hai nút nhấn, ST-Link và Keil µVision 5 với ARM Compiler 5.

| Thiết bị / tín hiệu | Chân STM32 | Ghi chú |
|---|---|---|
| MQ-2 AO | PA0 | ADC1, kênh 0 |
| OLED SCL | PB6 | I2C1 |
| OLED SDA | PB7 | I2C1; địa chỉ OLED mặc định `0x3C` |
| Điều khiển còi | PB11 | Mức thấp bật còi; dùng tầng đệm phù hợp với tải |
| Nút ALARM | PB12 | Nhấn nối xuống GND, sử dụng kéo lên nội |
| Nút SET | PB13 | Nhấn nối xuống GND, sử dụng kéo lên nội |
| GND | GND chung | Nối chung mass các khối |

Đầu vào ADC PA0 phải nằm trong khoảng **0–3,3 V**. MQ-2 cấp 5 V có thể xuất AO vượt 3,3 V, vì vậy cần mạch giới hạn/chia áp phù hợp và cập nhật `MQ2_AO_TO_ADC_GAIN` theo mạch thực tế. Khi demo logic, có thể dùng biến trở tạo điện áp trong dải ADC thay cho AO để thay đổi tín hiệu có kiểm soát. Tránh đưa tín hiệu đúng 0 V vì phần mềm hiện coi mẫu ADC bằng 0 là không hợp lệ.

### 2.2. Build và nạp chương trình

**Tình trạng mã nguồn:** repository đã có CMSIS, SPL và mã nguồn kernel FreeRTOS, nhưng hiện thiếu `main.c` ở thư mục gốc và `middlewares/FreeRTOS/config/FreeRTOSConfig.h`. Chưa có cấu hình chạy Ceedling. Cần bổ sung các thành phần này trước khi build hoặc chạy kiểm thử; các bước dưới đây áp dụng sau khi hoàn thiện phần khởi tạo.

1. Bổ sung `main.c`: khởi tạo board, tạo IPC, tạo bốn tác vụ, bật ngắt nút sau khi các thành phần liên quan đã sẵn sàng và gọi `vTaskStartScheduler()`. Kiểm tra kết quả tạo tài nguyên/tác vụ.
2. Bổ sung `FreeRTOSConfig.h` phù hợp với Cortex-M3 và ARMCC5; cấu hình tick 1 kHz, clock HSI 8 MHz, bật các API queue, mutex, semaphore, notification và software timer mà ứng dụng sử dụng. Ánh xạ các handler SVC, PendSV và SysTick theo port FreeRTOS.
3. Mở `MDK-ARM/BTL_N13.uvprojx`, chọn thiết bị **STM32F103C8** và **ARM Compiler 5**. Kiểm tra đường dẫn include và các file nguồn trong project.
4. Sử dụng scatter file `MDK-ARM/BTL_N13.sct`: Flash chương trình bắt đầu tại `0x08000000`, kích thước `0xFC00`; RAM bắt đầu tại `0x20000000`, kích thước `0x5000`. Trang Flash cuối 1 KB tại `0x0800FC00` dành cho lưu ngưỡng.
5. Bật **Create HEX File**, build bằng **F7** và xử lý hết lỗi trước khi nạp.
6. Kết nối ST-Link qua SWD, chọn thuật toán nạp STM32F10x phù hợp, đặt **BOOT0 = 0**, nạp chương trình rồi reset board.

Có thể tham khảo thêm [hướng dẫn Keil](MDK-ARM/KEIL_SETUP.md). Tài liệu này còn nhắc tới `scripts/setup_vendor.ps1`, `tests/VERIFICATION.md` và các tài liệu ngoài khác chưa có trong repository hiện tại; không thể chạy script đó từ bản mã nguồn này.

### 2.3. Kịch bản trình diễn

1. **Quan sát ban đầu:** cấp nguồn, chờ cảm biến ổn định. Khi chưa có mẫu hợp lệ, OLED hiển thị `WAIT SENSOR` / `MQ2 WARMUP`; đây là thông báo chờ dữ liệu, không phải bộ định thời làm nóng MQ-2. Với dữ liệu hợp lệ dưới ngưỡng, OLED hiển thị `SAFE`, PPM, ADC, hai ngưỡng và đồ thị lịch sử.
2. **Thử còi:** ở trạng thái `SAFE`, ngoài chế độ cài đặt, giữ ALARM khoảng 2 giây. Còi bật trong 1 giây và OLED hiển thị `TEST BUZZER`.
3. **Demo cảnh báo:** tăng tín hiệu analog hợp lệ để PPM lần lượt đạt ngưỡng WARNING và DANGER. Quan sát chữ trạng thái và nhịp còi chậm/nhanh. Giảm tín hiệu đủ thấp để quan sát hệ thống trở về trạng thái an toàn theo hysteresis.
4. **Tạm tắt còi:** khi đang WARNING hoặc DANGER, nhấn ngắn ALARM. Còi tắt trong 30 giây, OLED hiển thị thời gian còn lại. Nếu cảnh báo vẫn còn khi hết hạn, còi hoạt động trở lại; khi chuyển từ WARNING lên DANGER, chế độ tắt còi bị hủy.
5. **Đổi ngưỡng:** đưa hệ thống về `SAFE`, giữ SET khoảng 2 giây để chỉnh WARNING. Nhấn ngắn SET tăng 5 PPM, nhấn ngắn ALARM giảm 5 PPM. Giữ SET lần nữa để chuyển sang DANGER; giữ SET tiếp để lưu Flash và thoát. Hai ngưỡng phải cách nhau ít nhất 10 PPM, ngưỡng thấp nhất là 5 và giới hạn trên là 10000 PPM.
6. **Kiểm tra lưu cấu hình:** sau khi lưu thành công, reset board và kiểm tra hai ngưỡng được khôi phục. Nếu ghi Flash thất bại, OLED báo `FLASH ERROR`.
7. **Khôi phục mặc định:** khi `SAFE` và cảm biến có dữ liệu hợp lệ, giữ đồng thời ALARM + SET khoảng 2 giây để lưu lại ngưỡng 50/80.
8. **Demo lỗi dữ liệu:** dùng nguồn tín hiệu kiểm soát để tạo mẫu ADC không hợp lệ, chẳng hạn 0 V. Sau hơn 2 giây không nhận dữ liệu hợp lệ, OLED báo `SENSOR FAIL` và còi bật liên tục. Không dùng đầu ADC thả nổi để kiểm tra vì giá trị đọc không xác định.

## 3. Cây thư mục và chức năng từng nhánh

Cây dưới đây thể hiện các nhánh chính trong repository; các thư viện lớn được rút gọn.

```text
BTL_N13/
├── README.md                       # Tổng quan, hướng dẫn demo và cấu trúc dự án
├── .gitignore                      # Quy định các file không đưa vào Git
├── app/                            # Logic ứng dụng và các tác vụ FreeRTOS
│   ├── inc/                        # Cấu hình, kiểu dữ liệu và khai báo API ứng dụng
│   │   ├── app_config.h            # Ngưỡng, chu kỳ, thời gian và mô hình MQ-2
│   │   ├── app_ipc.h               # Queue, semaphore, trạng thái và mã sự kiện
│   │   └── task_*.h                # API tạo tác vụ và thông báo cho Alarm
│   └── src/                        # Phần triển khai logic ứng dụng
│       ├── app_ipc.c               # Tạo IPC, quản lý lịch sử 64 mẫu có mutex
│       ├── task_sensor.c           # Đọc/lọc ADC, ước lượng PPM, gửi dữ liệu
│       ├── task_alarm.c            # Phân loại, điều khiển còi, chỉnh/lưu ngưỡng
│       ├── task_button.c           # Chống dội, nhận nhấn ngắn/dài và tổ hợp nút
│       └── task_oled.c             # Hiển thị trạng thái, menu và đồ thị
├── bsp/                            # Giao diện dịch vụ board và ánh xạ chân
│   └── inc/
│       ├── bsp_board.h             # API ADC, nút, còi, I2C và lưu ngưỡng Flash
│       └── bsp_pins.h              # Chọn cổng/chân, kênh ADC và ngoại vi I2C
├── hal/                            # Hiện thực giao tiếp phần cứng và OLED
│   ├── inc/                        # Header GPIO, I2C, SSD1306 và cấu hình SPL
│   └── src/
│       ├── hal_stm32f103.c         # Clock HSI 8 MHz, ADC, Flash và dịch vụ BSP
│       ├── hal_gpio.c              # GPIO, còi, nút nhấn và cấu hình ngắt EXTI
│       ├── hal_i2c.c               # Khởi tạo I2C và truyền dữ liệu tới thiết bị
│       ├── ssd1306.c               # Driver OLED, chữ, số và các hàm vẽ
│       └── startup_stm32f10x_md.s  # Bảng vector và mã khởi động Cortex-M3
├── middlewares/                    # Thành phần hệ điều hành dùng chung
│   └── FreeRTOS/
│       ├── config/                 # Nơi đặt FreeRTOSConfig.h; hiện đang trống
│       └── Source/                 # Kernel FreeRTOS: task, queue, timer, event...
│           ├── include/            # Header API của FreeRTOS
│           └── portable/           # Phần phụ thuộc trình biên dịch và CPU
│               ├── MemMang/        # Các phương án quản lý heap; project dùng heap_4
│               └── RVDS/ARM_CM3/   # Port Cortex-M3 cho Keil/ARM Compiler 5
├── vendor/                         # Thư viện từ nhà cung cấp
│   ├── README.md                   # Ghi chú thư viện; có tham chiếu script còn thiếu
│   ├── CMSIS/
│   │   └── Include/                # Header lõi ARM Cortex, có core_cm3.h
│   └── STM32F10x_SPL/
│       ├── CMSIS/                  # Header hỗ trợ thiết bị STM32F10x
│       ├── inc/                    # Khai báo API Standard Peripheral Library
│       ├── src/                    # Triển khai driver ngoại vi SPL
│       ├── README.md               # Thông tin phần thư viện được đưa vào dự án
│       └── License.doc             # Điều khoản sử dụng thư viện
├── MDK-ARM/                        # Project và cấu hình công cụ Keil
│   ├── BTL_N13.uvprojx             # Danh sách nguồn và cấu hình build/debug
│   ├── BTL_N13.sct                 # Phân vùng Flash/RAM, chừa trang lưu ngưỡng
│   ├── KEIL_SETUP.md               # Hướng dẫn thiết lập Keil chi tiết
│   └── DebugConfig/                # Cấu hình phiên debug theo thiết bị
└── ceedling_tests/                 # Mã kiểm thử đơn vị trên máy tính
    └── test/
        ├── test_task_sensor.c      # Kiểm thử logic lấy mẫu và quy đổi cảm biến
        ├── test_task_alarm.c       # Kiểm thử logic cảnh báo và sự kiện
        └── support/               # Header FreeRTOS thay thế cho môi trường test
```

`inc/` chứa các khai báo để những module khác sử dụng; `src/` chứa mã triển khai. Khi đổi dây nối, cập nhật `bsp/inc/bsp_pins.h`. Khi đổi ngưỡng mặc định, chu kỳ hoặc hệ số cảm biến, cập nhật `app/inc/app_config.h`. Phần hiện thực dịch vụ BSP nằm trong `hal/src/hal_stm32f103.c`.
