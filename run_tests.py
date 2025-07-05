import serial
import time
import sys

DUT_PORT = 'COM15'
SIM_PORT = 'COM3'

DUT_BAUDRATE = 115200
SIM_BAUDRATE = 115200
TIMEOUT = 5  

def run_test(sim_serial, dut_serial, test_name, command, expected_output):
    
    print(f"\n--- Test Case: {test_name} ---")
    
    # Gửi lệnh đến Simulator
    print(f"Command: '{command.strip()}' -> Simulator: ({sim_serial.port})")
    sim_serial.write(command.encode('utf-8'))
    
    # Xóa bộ đệm đầu vào của DUT để đảm bảo chúng ta chỉ đọc dữ liệu mới
    dut_serial.reset_input_buffer()
    
    # Chờ DUT xử lý và trả về kết quả
    start_time = time.time()
    received_data = ""
    while time.time() - start_time < TIMEOUT:
        if dut_serial.in_waiting > 0:
            # Đọc một dòng từ DUT
            line = dut_serial.readline().decode('utf-8', errors='ignore').strip()
            if line: # Chỉ xử lý nếu dòng không rỗng
                print(f"DUT ({dut_serial.port}) return: '{line}'")
                received_data = line
                break 
        time.sleep(0.1) 

    # So sánh kết quả thực tế với kết quả kỳ vọng
    if expected_output in received_data:
        print(f"[PASS] - {test_name}")
        return True
    else:
        print(f"[FAIL] - {test_name}")
        print(f"Expected: '{expected_output}'")
        print(f"Received: '{received_data}'")
        return False

def main():
    """HIL Configuration"""

    sim_serial = None
    dut_serial = None

    try:
        sim_serial = serial.Serial(SIM_PORT, SIM_BAUDRATE, timeout=1)
        dut_serial = serial.Serial(DUT_PORT, DUT_BAUDRATE, timeout=1)
        time.sleep(2) 
        print("Serial Connected")
    except serial.SerialException as e:
        print(f"Error: {e}")
        sys.exit(1) 

    # --- Định nghĩa danh sách các kịch bản kiểm thử ---
    test_cases = [
        {
            "name": "(25.0°C, 60.0%)",
            "command": "SIM:25.0,60.0\n",
            "expected": "Temp: 25.0 C, Hum: 60.0 %"
        },
        {
            "name": "(35.5°C, 85.8%)",
            "command": "SIM:35.5,85.8\n",
            "expected": "Temp: 35.0 C, Hum: 85.0 %" 
        },
        {
            "name": "(0°C, 30.0%)",
            "command": "SIM:0.0,30.0\n",
            "expected": "Temp: 0.0 C, Hum: 30.0 %"
        },
        {
            "name": "Error: No Respond",
            "command": "FAIL\n",
            "expected": "Error: Failed to read from DHT sensor!"
        }
    ]

    results = []
    for test in test_cases:
        result = run_test(sim_serial, dut_serial, test["name"], test["command"], test["expected"])
        results.append(result)

    # Đóng cổng Serial sau khi hoàn tất để giải phóng tài nguyên
    if sim_serial and sim_serial.is_open:
        sim_serial.close()
    if dut_serial and dut_serial.is_open:
        dut_serial.close()
    print("\nSerial Disconnected.")


    # Tổng kết kết quả 
    if all(results):
        print("\n======================================")
        print("FINAL: PASS")
        print("======================================")
        sys.exit(0) # Trả về 0 cho pipeline biết là thành công
    else:
        failed_count = results.count(False)
        total_count = len(results)
        print("\n======================================")
        print(f"FINAL: FAIL ({failed_count}/{total_count})")
        print("======================================")
        sys.exit(1) # Trả về 1 cho pipeline biết là thất bại

if __name__ == "__main__":
    main()