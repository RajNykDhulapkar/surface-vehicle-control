import csv
from msilib.schema import Error
import time
import serial
import numpy as np
import matplotlib.pyplot as plt


# GLOBAL VARIABLES
SER_PORT = 'COM7'  # Serial port
SER_BAUD = 9600  # Serial baud rate
SAMPLE_FREQ = 10  # Frequency to record magnetometer readings at [Hz]
T_SAMPLE = 60 * 2  # Total time to read mangetometer readings [sec]
OUTPUT_FILENAME = 'mag-readings.txt'  # Output data file name


class SerialPort:
    """Create and read data from a serial port.

    Attributes:
        read(**kwargs): Read and decode data string from serial port.
    """

    def __init__(self, port, baud=9600):
        """Create and read serial data.

        Args:
            port (str): Serial port name. Example: 'COM4'
            baud (int): Serial baud rate, default 9600.
        """
        if isinstance(port, str) == False:
            raise TypeError('port must be a string.')

        if isinstance(baud, int) == False:
            raise TypeError('Baud rate must be an integer.')

        self.port = port
        self.baud = baud

        # Initialize serial connection
        self.ser = serial.Serial(self.port, self.baud, timeout=1)
        self.ser.flushInput()
        self.ser.flushOutput()

    def Read(self, clean_end=True):
        """
        Read and decode data string from serial port.

        Args:
            clean_end (bool): Strip '\\r' and '\\n' characters from string. Common if used Serial.println() Arduino function. Default true

        Returns:
            (str): utf-8 decoded message.
        """
        # while self.ser.in_waiting:
        try:
            bytesToRead = self.ser.readline()

            decodedMsg = bytesToRead.decode('utf-8')

            if clean_end == True:
                decodedMsg = decodedMsg.strip('\r').strip(
                    '\n')  # Strip extra chars at the end

            return decodedMsg
        except:
            raise Error()

    def Write(self, msg):
        """
        Write string to serial port.

        Args
        ----
            msg (str): Message to transmit

        Returns
        -------
            (bool) True if successful, false if not
        """
        try:
            self.ser.write(msg.encode())
            return True
        except:
            print("Error sending message.")

    def Close(self):
        """Close serial connection."""
        self.ser.close()


class PlotPoints3D:
    """Plot magnetometer readings as 3D scatter plot.

    Attributes:
        AddPoint(x, y, z): Add 3D point to plot.

    """

    def __init__(self, fig, ax, live_plot=True, marker='o', c='r'):
        self.fig = fig  # fig and axes
        self.ax = ax
        self.live_plot = live_plot
        self.ptMarker = marker  # Point symbol
        self.ptColor = c  # Point color
        self.edgeColor = 'k'  # Border/edge of point
        self.ax.set_xlim((-80, 80))  # Set axes limits to keep plot shape
        self.ax.set_ylim((-80, 80))
        self.ax.set_zlim((-80, 80))

    def AddPoint(self, x, y, z):
        """Add 3D point to scatter plot.

        Args:
            x (float): X-coordinate.
            y (float): Y-coordinate.
            z (float): Z-coordinate.
        """
        self.ax.scatter(x, y, z, marker=self.ptMarker,
                        s=10,
                        c=self.ptColor, edgecolors=self.edgeColor)

        if self.live_plot == True:
            plt.pause(0.001)


Arduino = SerialPort(SER_PORT, SER_BAUD)
N = int(SAMPLE_FREQ * T_SAMPLE)  # Number of readings
DT = 1.0 / SAMPLE_FREQ  # Sample period [sec]


# Create live plot for logging points
fig_rawReadings = plt.figure(figsize=(10, 8))
ax_rawReadings = fig_rawReadings.add_subplot(111, projection='3d')
RawDataPlot = PlotPoints3D(fig_rawReadings, ax_rawReadings, live_plot=False)


# Take a few readings to 'flush' out bad ones
for _ in range(4):
    data = Arduino.Read().split(',')  # Split into separate values
    time.sleep(0.25)


measurements = np.zeros((N, 3), dtype='float')

for currMeas in range(N):
    try:
        data = Arduino.Read().split(',')  # Split into separate values

        if (len(data) == 3):
            mx, my, mz = float(data[0]), float(data[1]), float(
                data[2])  # Convert to floats, [uT]
        else:
            print("shot")
            continue

        print('[%0.4f, %0.4f, %0.4f] uT  |  Norm: %0.4f uT  |  %0.1f %% Complete.' %
              (mx, my, mz, np.sqrt(mx**2 + my**2 + mz**2), (currMeas / N) * 100.0)
              )

        # Store data to array
        measurements[currMeas, 0] = mx
        measurements[currMeas, 1] = my
        measurements[currMeas, 2] = mz

        RawDataPlot.AddPoint(mx, my, mz)  # Add point to 3D plot

    except:
        print("shot")
        continue


# After measurements are complete, write data to file
Arduino.Close()
print('Sensor Reading Complete!')

print('Writing data to {} ...'.format(OUTPUT_FILENAME))
for i in range(N):
    with open(OUTPUT_FILENAME, 'a', newline='') as f:
        writer = csv.writer(f, delimiter='\t')
        writer.writerow(
            [measurements[i, 0], measurements[i, 1], measurements[i, 2]])

plt.show()
