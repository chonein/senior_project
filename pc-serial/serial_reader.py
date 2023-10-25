import serial

BAUD_RATE = 115_200


def button_is_pressed(dev_file):
    with serial.Serial(dev_file, BAUD_RATE, timeout=0) as ser:
        while True:
            try:
                data = ser.read(1)  # Read one byte of binary data
                if data:
                    # You can process or display the binary data as needed
                    print(data, end='', flush=True)
            except KeyboardInterrupt:
                print("\nExiting...")
                break
    #     data = ser.read(1)
    #     if (len(data) != 0):
    #         print(data, len(data))
        # print(ser.readall())


# while True:
button_is_pressed('/dev/tty.usbmodem1101')
