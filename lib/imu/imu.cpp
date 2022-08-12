#include "Arduino.h"
#include "imu.h"

/* MPU9250 object, input the I2C bus and address */
IMU::IMU(TwoWire &bus, uint8_t address)
{
    _i2c = &bus;        // I2C bus
    _address = address; // I2C address
}

/* starts communication with the MPU-9250 */
int IMU::begin()

{

    // using I2C for communication
    // starting the I2C bus
    _i2c->begin();
    // setting the I2C clock
    _i2c->setClock(_i2cRate);

    // select clock source to gyro
    if (writeRegister(PWR_MGMNT_1, CLOCK_SEL_PLL) < 0)
    {
        return -1;
    }
    // enable I2C master mode
    if (writeRegister(USER_CTRL, I2C_MST_EN) < 0)
    {
        return -2;
    }
    // set the I2C bus speed to 400 kHz
    if (writeRegister(I2C_MST_CTRL, I2C_MST_CLK) < 0)
    {
        return -3;
    }
    // set AK8963 to Power Down
    writeAK8963Register(AK8963_CNTL1, AK8963_PWR_DOWN);
    // reset the MPU9250
    writeRegister(PWR_MGMNT_1, PWR_RESET);
    // wait for MPU-9250 to come back up
    delay(1);
    // reset the AK8963
    writeAK8963Register(AK8963_CNTL2, AK8963_RESET);
    // select clock source to gyro
    if (writeRegister(PWR_MGMNT_1, CLOCK_SEL_PLL) < 0)
    {
        return -4;
    }
    // check the WHO AM I byte, expected value is 0x71 (decimal 113) or 0x73 (decimal 115)
    if ((whoAmI() != 113) && (whoAmI() != 115))
    {
        return -5;
    }
    // enable accelerometer and gyro
    if (writeRegister(PWR_MGMNT_2, SEN_ENABLE) < 0)
    {
        return -6;
    }
    // setting accel range to 16G as default
    if (writeRegister(ACCEL_CONFIG, ACCEL_FS_SEL_16G) < 0)
    {
        return -7;
    }
    _accelScale = G * 16.0f / 32767.5f; // setting the accel scale to 16G
    _accelRange = ACCEL_RANGE_16G;
    // setting the gyro range to 2000DPS as default
    if (writeRegister(GYRO_CONFIG, GYRO_FS_SEL_2000DPS) < 0)
    {
        return -8;
    }
    _gyroScale = 2000.0f / 32767.5f * _d2r; // setting the gyro scale to 2000DPS
    _gyroRange = GYRO_RANGE_2000DPS;
    // setting bandwidth to 184Hz as default
    if (writeRegister(ACCEL_CONFIG2, ACCEL_DLPF_184) < 0)
    {
        return -9;
    }
    if (writeRegister(CONFIG, GYRO_DLPF_184) < 0)
    { // setting gyro bandwidth to 184Hz
        return -10;
    }
    _bandwidth = DLPF_BANDWIDTH_184HZ;
    // setting the sample rate divider to 0 as default
    if (writeRegister(SMPDIV, 0x00) < 0)
    {
        return -11;
    }
    _srd = 0;
    // enable I2C master mode
    if (writeRegister(USER_CTRL, I2C_MST_EN) < 0)
    {
        return -12;
    }
    // set the I2C bus speed to 400 kHz
    if (writeRegister(I2C_MST_CTRL, I2C_MST_CLK) < 0)
    {
        return -13;
    }
    // check AK8963 WHO AM I register, expected value is 0x48 (decimal 72)
    if (whoAmIAK8963() != 72)
    {
        return -14;
    }
    /* get the magnetometer calibration */
    // set AK8963 to Power Down
    if (writeAK8963Register(AK8963_CNTL1, AK8963_PWR_DOWN) < 0)
    {
        return -15;
    }
    delay(100); // long wait between AK8963 mode changes
    // set AK8963 to FUSE ROM access
    if (writeAK8963Register(AK8963_CNTL1, AK8963_FUSE_ROM) < 0)
    {
        return -16;
    }
    delay(100); // long wait between AK8963 mode changes
    // read the AK8963 ASA registers and compute magnetometer scale factors
    readAK8963Registers(AK8963_ASA, 3, _buffer);
    _magScaleX = ((((float)_buffer[0]) - 128.0f) / (256.0f) + 1.0f) * 4912.0f / 32760.0f; // micro Tesla
    _magScaleY = ((((float)_buffer[1]) - 128.0f) / (256.0f) + 1.0f) * 4912.0f / 32760.0f; // micro Tesla
    _magScaleZ = ((((float)_buffer[2]) - 128.0f) / (256.0f) + 1.0f) * 4912.0f / 32760.0f; // micro Tesla
    // set AK8963 to Power Down
    if (writeAK8963Register(AK8963_CNTL1, AK8963_PWR_DOWN) < 0)
    {
        return -17;
    }
    delay(100); // long wait between AK8963 mode changes
    // set AK8963 to 16 bit resolution, 100 Hz update rate
    if (writeAK8963Register(AK8963_CNTL1, AK8963_CNT_MEAS2) < 0)
    {
        return -18;
    }
    delay(100); // long wait between AK8963 mode changes
    // select clock source to gyro
    if (writeRegister(PWR_MGMNT_1, CLOCK_SEL_PLL) < 0)
    {
        return -19;
    }
    // instruct the MPU9250 to get 7 bytes of data from the AK8963 at the sample rate
    readAK8963Registers(AK8963_HXL, 7, _buffer);
    // estimate gyro bias

    // successful init, return 1
    return 1;
}

/* sets the accelerometer full scale range to values other than default */
int IMU::setAccelRange(AccelRange range)
{
    switch (range)
    {
    case ACCEL_RANGE_2G:
    {
        // setting the accel range to 2G
        if (writeRegister(ACCEL_CONFIG, ACCEL_FS_SEL_2G) < 0)
        {
            return -1;
        }
        _accelScale = G * 2.0f / 32767.5f; // setting the accel scale to 2G
        break;
    }
    case ACCEL_RANGE_4G:
    {
        // setting the accel range to 4G
        if (writeRegister(ACCEL_CONFIG, ACCEL_FS_SEL_4G) < 0)
        {
            return -1;
        }
        _accelScale = G * 4.0f / 32767.5f; // setting the accel scale to 4G
        break;
    }
    case ACCEL_RANGE_8G:
    {
        // setting the accel range to 8G
        if (writeRegister(ACCEL_CONFIG, ACCEL_FS_SEL_8G) < 0)
        {
            return -1;
        }
        _accelScale = G * 8.0f / 32767.5f; // setting the accel scale to 8G
        break;
    }
    case ACCEL_RANGE_16G:
    {
        // setting the accel range to 16G
        if (writeRegister(ACCEL_CONFIG, ACCEL_FS_SEL_16G) < 0)
        {
            return -1;
        }
        _accelScale = G * 16.0f / 32767.5f; // setting the accel scale to 16G
        break;
    }
    }
    _accelRange = range;
    return 1;
}

/* sets the gyro full scale range to values other than default */
int IMU::setGyroRange(GyroRange range)
{
    switch (range)
    {
    case GYRO_RANGE_250DPS:
    {
        // setting the gyro range to 250DPS
        if (writeRegister(GYRO_CONFIG, GYRO_FS_SEL_250DPS) < 0)
        {
            return -1;
        }
        _gyroScale = 250.0f / 32767.5f * _d2r; // setting the gyro scale to 250DPS
        break;
    }
    case GYRO_RANGE_500DPS:
    {
        // setting the gyro range to 500DPS
        if (writeRegister(GYRO_CONFIG, GYRO_FS_SEL_500DPS) < 0)
        {
            return -1;
        }
        _gyroScale = 500.0f / 32767.5f * _d2r; // setting the gyro scale to 500DPS
        break;
    }
    case GYRO_RANGE_1000DPS:
    {
        // setting the gyro range to 1000DPS
        if (writeRegister(GYRO_CONFIG, GYRO_FS_SEL_1000DPS) < 0)
        {
            return -1;
        }
        _gyroScale = 1000.0f / 32767.5f * _d2r; // setting the gyro scale to 1000DPS
        break;
    }
    case GYRO_RANGE_2000DPS:
    {
        // setting the gyro range to 2000DPS
        if (writeRegister(GYRO_CONFIG, GYRO_FS_SEL_2000DPS) < 0)
        {
            return -1;
        }
        _gyroScale = 2000.0f / 32767.5f * _d2r; // setting the gyro scale to 2000DPS
        break;
    }
    }
    _gyroRange = range;
    return 1;
}

/* reads the most current data from MPU9250 and stores in buffer */
int IMU::readSensor()
{
    // grab the data from the MPU9250
    if (readRegisters(ACCEL_OUT, 21, _buffer) < 0)
    {
        return -1;
    }
    // combine into 16 bit values
    _axcounts = (((int16_t)_buffer[0]) << 8) | _buffer[1];
    _aycounts = (((int16_t)_buffer[2]) << 8) | _buffer[3];
    _azcounts = (((int16_t)_buffer[4]) << 8) | _buffer[5];
    _tcounts = (((int16_t)_buffer[6]) << 8) | _buffer[7];
    _gxcounts = (((int16_t)_buffer[8]) << 8) | _buffer[9];
    _gycounts = (((int16_t)_buffer[10]) << 8) | _buffer[11];
    _gzcounts = (((int16_t)_buffer[12]) << 8) | _buffer[13];
    _hxcounts = (((int16_t)_buffer[15]) << 8) | _buffer[14];
    _hycounts = (((int16_t)_buffer[17]) << 8) | _buffer[16];
    _hzcounts = (((int16_t)_buffer[19]) << 8) | _buffer[18];
    // transform and convert to float values
    _ax = (((float)(tX[0] * _axcounts + tX[1] * _aycounts + tX[2] * _azcounts) * _accelScale));
    _ay = (((float)(tY[0] * _axcounts + tY[1] * _aycounts + tY[2] * _azcounts) * _accelScale));
    _az = (((float)(tZ[0] * _axcounts + tZ[1] * _aycounts + tZ[2] * _azcounts) * _accelScale));
    _gx = ((float)(tX[0] * _gxcounts + tX[1] * _gycounts + tX[2] * _gzcounts) * _gyroScale);
    _gy = ((float)(tY[0] * _gxcounts + tY[1] * _gycounts + tY[2] * _gzcounts) * _gyroScale);
    _gz = ((float)(tZ[0] * _gxcounts + tZ[1] * _gycounts + tZ[2] * _gzcounts) * _gyroScale);
    _hx = (((float)(_hxcounts)*_magScaleX));
    _hy = (((float)(_hycounts)*_magScaleY));
    _hz = (((float)(_hzcounts)*_magScaleZ));
    _t = ((((float)_tcounts) - _tempOffset) / _tempScale) + _tempOffset;
    return 1;
}

/* returns the accelerometer measurement in the x direction, m/s/s */
float IMU::getAccelX_mss()
{
    return _ax;
}

/* returns the accelerometer measurement in the y direction, m/s/s */
float IMU::getAccelY_mss()
{
    return _ay;
}

/* returns the accelerometer measurement in the z direction, m/s/s */
float IMU::getAccelZ_mss()
{
    return _az;
}

/* returns the gyroscope measurement in the x direction, rad/s */
float IMU::getGyroX_rads()
{
    return _gx;
}

/* returns the gyroscope measurement in the y direction, rad/s */
float IMU::getGyroY_rads()
{
    return _gy;
}

/* returns the gyroscope measurement in the z direction, rad/s */
float IMU::getGyroZ_rads()
{
    return _gz;
}

/* returns the magnetometer measurement in the x direction, uT */
float IMU::getMagX_uT()
{
    return _hx;
}

/* returns the magnetometer measurement in the y direction, uT */
float IMU::getMagY_uT()
{
    return _hy;
}

/* returns the magnetometer measurement in the z direction, uT */
float IMU::getMagZ_uT()
{
    return _hz;
}

/* returns the die temperature, C */
float IMU::getTemperature_C()
{
    return _t;
}

/* writes a byte to MPU9250 register given a register address and data */
int IMU::writeRegister(uint8_t subAddress, uint8_t data)
{

    _i2c->beginTransmission(_address); // open the device
    _i2c->write(subAddress);           // write the register address
    _i2c->write(data);                 // write the data
    _i2c->endTransmission();

    delay(10);

    /* read back the register */
    readRegisters(subAddress, 1, _buffer);
    /* check the read back register against the written register */
    if (_buffer[0] == data)
    {
        return 1;
    }
    else
    {
        return -1;
    }
}

/* reads registers from MPU9250 given a starting register address, number of bytes, and a pointer to store data */
int IMU::readRegisters(uint8_t subAddress, uint8_t count, uint8_t *dest)
{
    _i2c->beginTransmission(_address); // open the device
    _i2c->write(subAddress);           // specify the starting register address
    _i2c->endTransmission(false);
    _numBytes = _i2c->requestFrom(_address, count); // specify the number of bytes to receive
    if (_numBytes == count)
    {
        for (uint8_t i = 0; i < count; i++)
        {
            dest[i] = _i2c->read();
        }
        return 1;
    }
    else
    {
        return -1;
    }
}

/* writes a register to the AK8963 given a register address and data */
int IMU::writeAK8963Register(uint8_t subAddress, uint8_t data)
{
    // set slave 0 to the AK8963 and set for write
    if (writeRegister(I2C_SLV0_ADDR, AK8963_I2C_ADDR) < 0)
    {
        return -1;
    }
    // set the register to the desired AK8963 sub address
    if (writeRegister(I2C_SLV0_REG, subAddress) < 0)
    {
        return -2;
    }
    // store the data for write
    if (writeRegister(I2C_SLV0_DO, data) < 0)
    {
        return -3;
    }
    // enable I2C and send 1 byte
    if (writeRegister(I2C_SLV0_CTRL, I2C_SLV0_EN | (uint8_t)1) < 0)
    {
        return -4;
    }
    // read the register and confirm
    if (readAK8963Registers(subAddress, 1, _buffer) < 0)
    {
        return -5;
    }
    if (_buffer[0] == data)
    {
        return 1;
    }
    else
    {
        return -6;
    }
}

/* reads registers from the AK8963 */
int IMU::readAK8963Registers(uint8_t subAddress, uint8_t count, uint8_t *dest)
{
    // set slave 0 to the AK8963 and set for read
    if (writeRegister(I2C_SLV0_ADDR, AK8963_I2C_ADDR | I2C_READ_FLAG) < 0)
    {
        return -1;
    }
    // set the register to the desired AK8963 sub address
    if (writeRegister(I2C_SLV0_REG, subAddress) < 0)
    {
        return -2;
    }
    // enable I2C and request the bytes
    if (writeRegister(I2C_SLV0_CTRL, I2C_SLV0_EN | count) < 0)
    {
        return -3;
    }
    delay(1); // takes some time for these registers to fill
              // read the bytes off the MPU9250 EXT_SENS_DATA registers
    _status = readRegisters(EXT_SENS_DATA_00, count, dest);
    return _status;
}

/* gets the MPU9250 WHO_AM_I register value, expected to be 0x71 */
int IMU::whoAmI()
{
    // read the WHO AM I register
    if (readRegisters(WHO_AM_I, 1, _buffer) < 0)
    {
        return -1;
    }
    // return the register value
    return _buffer[0];
}

/* gets the AK8963 WHO_AM_I register value, expected to be 0x48 */
int IMU::whoAmIAK8963()
{
    // read the WHO AM I register
    if (readAK8963Registers(AK8963_WHO_AM_I, 1, _buffer) < 0)
    {
        return -1;
    }
    // return the register value
    return _buffer[0];
}

bool IMU::getHeading(double *psi)
{
    float magX = getMagX_uT() - 19.03355;
    float magY = getMagY_uT() - 27.47569;
    float magZ = getMagZ_uT() - 66.334158;

    magX = 0.89534 * magX - 0.017772 * magY - 0.01735 * magZ;
    magY = -0.017772 * magX + 0.883792 * magY - 0.022609;

    if ((magX > 0 && magY > 0) || (magX < 0 && magY > 0))
        if ((360 - (atan2(magY, magX) * 180 / PI)) > 270)
            *psi = (360 - (atan2(magY, magX) * 180 / PI)) - 270;
        else
            *psi = (360 - (atan2(magY, magX) * 180 / PI)) + 90;
    else
        *psi = (360 - (atan2(magY, magX) * 180 / PI + 360)) + 90;
    return false;
}
