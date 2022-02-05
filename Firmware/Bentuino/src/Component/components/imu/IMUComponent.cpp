bool IMUComponent::initInternal()
{
    isConnected = addParameter("connected", false);
    sendLevel = addParameter("sendLevel", false);
    orientationSendRate = addParameter("orientationSendRate", 50);

    int sdaPin = GetIntConfig("sdaPin");
    int sclPin = GetIntConfig("sclPin");

    if (sdaPin == 0)
    {
        sdaPin = 23;
        SetConfig("sdaPin", sdaPin);
    }

    if (sclPin == 0)
    {
        sclPin = 22;
        SetConfig("sclPin", sclPin);
    }

    Wire.begin(sdaPin, sclPin);

    if (!bno.begin())
    {
        NDBG("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
        return false;
    }

    bno.setMode(Adafruit_BNO055::OPERATION_MODE_CONFIG);
    bno.setAxisRemap(Adafruit_BNO055::REMAP_CONFIG_P0);
    bno.setAxisSign(Adafruit_BNO055::REMAP_SIGN_P0);
    bno.setMode(Adafruit_BNO055::OPERATION_MODE_NDOF);
    bno.setExtCrystalUse(true);
    bno.enterNormalMode();

    // Init values
    accelThresholds[0] = .8f;
    accelThresholds[1] = 2;
    accelThresholds[2] = 4;

    diffThreshold = 8;

    flatThresholds[0] = .8f;
    flatThresholds[1] = 2;

    semiFlatThreshold = 2;
    loftieThreshold = 12;
    singleThreshold = 25;

    activity = .0;
    prevActivity = .0;
    orientationXOffset = .0;

    angleOffset = .0f;

    timeSinceOrientationLastSent = 0;
    throwState = 0;

#ifdef IMU_READ_ASYNC
    hasNewData = false,
    shouldStopRead = false;
    imuLock = false;
    xTaskCreate(&IMUComponent::readIMUStatic, "imu", NATIVE_STACK_SIZE, this, 1, NULL);
#endif

    return true;
}

void IMUComponent::updateInternal()
{
#ifdef IMU_READ_ASYNC
    if (!hasNewData)
        return;
    hasNewData = false;
    imuLock = true;
#else
    readIMU();
#endif

    long curTime = millis();
    int orientationSendMS = 1000 / (float)orientationSendRate->val;

    if (curTime > timeSinceOrientationLastSent + orientationSendMS)
    {
        if ((int)sendLevel->val >= 1)
        {
           //NDBG("Orientation send "+
           sendEvent(OrientationUpdate, orientation, 3);
            if ((int)sendLevel->val >= 2)
            {
                sendEvent(AccelUpdate, accel, 3);
                sendEvent(LinearAccelUpdate, linearAccel, 3);
                sendEvent(GyroUpdate, gyro, 3);
                sendEvent(ActivityUpdate);
                sendEvent(ProjectedAngleUpdate);
                // sendEvent(Gravity, gravity, 3);
            }
        }

        timeSinceOrientationLastSent = curTime;
    }

#ifdef IMU_READ_ASYNC
    imuLock = false;
#endif
}

void IMUComponent::clearInternal()
{
#ifdef IMU_READ_ASYNC
    shouldStopRead = true;
#endif
}

#ifdef IMU_READ_ASYNC
void IMUComponent::readIMUStatic(void *_imu)
{
    DBG("[IMU] Async Thread Start");
    IMUComponent *imuComp = (IMUComponent *)_imu;
    while (!imuComp->shouldStopRead)
    {
        imuComp->readIMU();
        delay(5);
    }

    DBG("[IMU] Async Thread Stop");

    vTaskDelete(NULL);

    DBG("[IMU] Task deleted");
}
#endif

void IMUComponent::readIMU()
{
    if(!enabled->val) return;

#ifdef IMU_READ_ASYNC
    if (imuLock)
        return;
#endif

    imu::Quaternion q = bno.getQuat();
    q.normalize();

    imu::Vector<3> euler = q.toEuler();
    orientation[0] = (float)(fmodf(((euler.x() * 180 / PI) + orientationXOffset + 180.0f * 5), 360.0f) - 180.0f);
    orientation[1] = (float)(euler.y() * 180 / PI); // Pitch
    orientation[2] = (float)(euler.z() * 180 / PI); // Roll

    imu::Vector<3> acc = bno.getVector(Adafruit_BNO055::VECTOR_ACCELEROMETER);
    accel[0] = (float)acc.x();
    accel[1] = (float)acc.y();
    accel[2] = (float)acc.z();

    imu::Vector<3> laccel = bno.getVector(Adafruit_BNO055::VECTOR_LINEARACCEL);
    linearAccel[0] = (float)laccel.x();
    linearAccel[1] = (float)laccel.y();
    linearAccel[2] = (float)laccel.z();

    imu::Vector<3> gyr = bno.getVector(Adafruit_BNO055::VECTOR_GYROSCOPE);
    gyro[0] = (float)gyr.x();
    gyro[1] = (float)gyr.y();
    gyro[2] = (float)gyr.z();

    // imu::Vector<3> grav = bno.getVector(Adafruit_BNO055::VECTOR_GRAVITY);
    // gravity[0] = grav.x();
    // gravity[1] = grav.y();
    // gravity[2] = grav.z();

    computeThrow();
    computeActivity();
    computeProjectedAngle();

#ifdef IMU_READ_ASYNC
    hasNewData = true;
#endif
}

void IMUComponent::computeProjectedAngle()
{
    float eulerRadians[3];
    float lookAt[3];
    float result;

    // Recalculate x orientation for the projected angle, based on xOnCalibration
    float xOrientation = orientation[0];
    float newX = 0;
    if (xOnCalibration < 0)
    {
        if (xOrientation > xOnCalibration)
        {
            if (xOrientation < 0)
            {
                newX = (xOnCalibration * -1) - (xOrientation * -1);
            }
            else
            {
                if (xOrientation + (xOnCalibration * -1) > 180.0f)
                {
                    newX = (360.0f - xOrientation - (xOnCalibration * -1)) * -1;
                }
                else
                {
                    newX = xOrientation + (xOnCalibration * -1);
                }
            }
        }
        else
        {
            newX = (xOnCalibration * -1) - (xOrientation * -1);
        }
    }
    else
    {
        if (xOrientation > xOnCalibration)
        {
            newX = xOrientation - xOnCalibration;
        }
        else
        {
            if ((xOrientation - xOnCalibration) < -180)
            {
                newX = (180.0f - xOnCalibration) + (180.0f - (xOrientation * -1));
            }
            else
            {
                newX = xOrientation - xOnCalibration;
            }
        }
    }

    eulerRadians[0] = newX * PI / 180.0f;
    eulerRadians[1] = (float)orientation[1] * PI / 180.0f;
    eulerRadians[2] = (float)orientation[2] * PI / 180.0f;

    lookAt[0] = cos(eulerRadians[1]) * sin(eulerRadians[0]);
    lookAt[1] = sin(eulerRadians[1]);
    lookAt[2] = cos(eulerRadians[1]) * cos(eulerRadians[0]);

    result = atan(lookAt[1] / lookAt[2]) - PI / 2.0f;

    if (lookAt[2] > 0.0f)
    {
        result = result + PI;
    }

    result = (result / PI) * 0.5f + 0.5f;
    result = fmod((result + angleOffset), 1.0f);

    projectedAngle = result;
}

void IMUComponent::computeThrow()
{
    float maxAccelYZ = max(fabsf(accel[1]), fabsf(accel[2]));
    float maxAccel = max(maxAccelYZ, fabsf(accel[0]));
    float maxLinearAccel = max(max(fabsf(linearAccel[0]), fabsf(linearAccel[1])), fabsf(linearAccel[2]));
    float accLinearDiff = fabsf(maxAccelYZ - maxLinearAccel);

    bool curIsFlat = throwState == 1;
    float flatThresh = curIsFlat ? flatThresholds[1] : flatThresholds[0];
    bool isFlatting = maxAccel < flatThresh;
    bool isFastSpin = false;

    int newState = 0;
    if (isFlatting)
    {
        newState = 1; // flat
    }
    else
    {
        bool curIsThrowing = throwState > 1;
        float throwThresh = curIsThrowing ? accelThresholds[1] : accelThresholds[0];
        throwThresh = isFastSpin ? throwThresh : accelThresholds[2];

        bool accelCheck = maxAccelYZ < throwThresh;

        bool isThrowing = false;

        if (curIsThrowing)
        {
            isThrowing = accelCheck;
        }
        else
        {
            bool diffCheck = accLinearDiff > diffThreshold;
            isThrowing = accelCheck && diffCheck;
        }

        float throwPower = fabsf(accel[0]);
        if (isThrowing)
        {
            if (throwPower < semiFlatThreshold)
                newState = 4;
            else if (throwPower < loftieThreshold)
                newState = 5;
            else if (throwPower < singleThreshold)
                newState = 2;
            else
                newState = 3; // double
        }
    }

    if (newState != throwState)
    {
        throwState = newState;
        sendEvent(ThrowState);
    }
}

void IMUComponent::computeActivity()
{
    float maxLinearAccel = max(max(fabsf(linearAccel[0]), fabsf(linearAccel[1])), fabsf(linearAccel[2]));
    maxLinearAccel = (((maxLinearAccel - 0) * (1 - 0)) / (40 - 0)) + 0; // remap to 0..1 range
    maxLinearAccel = min(maxLinearAccel, (float)1.0);

    activity = prevActivity + (maxLinearAccel - prevActivity) * 0.1;
    prevActivity = activity;
}

void IMUComponent::sendCalibrationStatus()
{
    uint8_t system, gyro, accel, mag;
    system = gyro = accel = mag = 0;
    bno.getCalibration(&system, &gyro, &accel, &mag);

    calibration[0] = (float)system;
    calibration[1] = (float)gyro;
    calibration[2] = (float)accel;
    calibration[3] = (float)mag;

    sendEvent(CalibrationStatus, calibration, 4);
}

void IMUComponent::setOrientationXOffset(float offset = 0.0f)
{
    orientationXOffset = offset;
}

void IMUComponent::setProjectAngleOffset(float yaw = 0.0f, float angle = 0.0f)
{
    angleOffset = angle;
    xOnCalibration = yaw;
}

bool IMUComponent::handleCommandInternal(const String& command, var *data, int numData)
{
    if (checkCommand(command, "calibrationStatus", numData, 0))
    {
        sendCalibrationStatus();
        return true;
    }

    return false;
}