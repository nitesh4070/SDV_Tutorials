#include <mcp_can.h>
#include <SPI.h>

MCP_CAN CAN0(5);

unsigned long rxId;
byte len;
byte rxBuf[8];

int vehicleSpeed = 60;
int speedLimit = 60;

String inputBuffer = "";

void setup()
{
    Serial.begin(115200);

    while (CAN_OK != CAN0.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ))
    {
        Serial.println("CAN Init Failed");
        delay(1000);
    }

    CAN0.setMode(MCP_NORMAL);

    Serial.println("AEB ECU Ready");
    Serial.println("Type Vehicle Speed and Press ENTER");  // setting the vehilce speed. you feel like you are manually simulating it.
}

void loop()
{
    // -------- SAFE SERIAL INPUT --------
    while (Serial.available())
    {
        char c = Serial.read();

        if (c == '\n')
        {
            int newSpeed = inputBuffer.toInt();

            // if the correct spped value is entered then only it will update
            if(newSpeed > 0)
            {
                vehicleSpeed = newSpeed;

                Serial.print("Vehicle Speed Updated: ");
                Serial.println(vehicleSpeed);
            }

            inputBuffer = "";
        }
        else if (c != '\r')
        {
            inputBuffer += c;
        }
    }

    // -------- RECEIVE TRAFFIC SIGN --------  /this can be simulated later with camera's inputs
    if(CAN0.checkReceive() == CAN_MSGAVAIL)
    {
        CAN0.readMsgBuf(&rxId, &len, rxBuf);

        if(rxId == 0x100)
        {
            speedLimit = rxBuf[0];

            Serial.println("----------------");

            Serial.print("Speed Limit: ");
            Serial.println(speedLimit);

            Serial.print("Vehicle Speed: ");
            Serial.println(vehicleSpeed);

            int difference = vehicleSpeed - speedLimit;

            byte aebState = 0;

            if(difference > 20)
            {
                Serial.println("AEB BRAKING ACTIVATED");
                aebState = 2;
            }
            else if(difference > 10)
            {
                Serial.println("WARNING: Reduce Speed");
                aebState = 1;
            }
            else
            {
                Serial.println("Vehicle Speed SAFE");
                aebState = 0;
            }

            // -------- SEND AEB STATUS --------
            byte txData[8] = {aebState,0,0,0,0,0,0,0};

            byte status = CAN0.sendMsgBuf(0x200, 0, 8, txData);

            if(status == CAN_OK)
            {
                Serial.print("AEB Status Sent: ");
                Serial.println(aebState);
            }
            else
            {
                Serial.println("AEB TX Failed");
            }
        }
    }

    delay(100);
}