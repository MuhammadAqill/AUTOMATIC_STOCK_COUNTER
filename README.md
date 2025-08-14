
## Automatic Stock Counter

**Automatic Stock Counter** is a Final Year Project (FYP) that automates the stock counting process using electronics (ESP-IDF microcontroller) with a simple user interface.

## Key Features
- Automatically detects and counts stock  
- Developed using **ESP-IDF**, **C**, and **CMake**  
- Modular structure (components organized under `/components` and `/main`)  
- Suitable for IoT system integration or inventory automation  

## Project Structure
```text
├── components/           # Modules and libraries
├── main/                 # Main project source code
├── CMakeLists.txt        # Project build configuration
├── sdkconfig             # ESP-IDF configuration file
└── README.md             # Project documentation
````

## Preparation & How to Build

1. Install **ESP-IDF** and set up the recommended development environment.
2. Clone this repository:

   ```bash
   git clone https://github.com/MuhammadAqill/AUTOMATIC_STOCK_COUNTER.git
   ```
3. Navigate to the project directory:

   ```bash
   cd AUTOMATIC_STOCK_COUNTER
   ```
4. Build and flash the firmware to the ESP board:

   ```bash
   idf.py build
   idf.py -p (PORT_SERIAL) flash monitor
   ```

## Usage

1. Deploy the project to the ESP board with connections to:

   * Parent board
   * Stock sensor (e.g., object counter sensor)
2. Run the firmware; the board will start counting stock automatically.
3. Monitor the serial output for:

   * Current stock counting status
   * Notifications when stock reaches a threshold

## Suggested Improvements (Roadmap)

* Add **Wi-Fi/MQTT** support for remote stock data transmission
* Create a **web interface** or mobile app for real-time access
* Support more sensor types (e.g., RFID, IR, ultrasonic)
* Log stock count data to **SD Card** or cloud storage
* Integrate with ERP systems for automated ordering

## Author

**Muhammad Aqil**
This FYP project was developed in 2025.
