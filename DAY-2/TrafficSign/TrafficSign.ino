#include <mcp_can.h>
#include <SPI.h>

MCP_CAN CAN0(5);

int speedLimit = 60;
String inputString = "";

void setup()
{
    Serial.begin(115200);
    delay(1000);

    while (CAN_OK != CAN0.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ))
    {
        Serial.println("CAN Init Failed...");
        delay(1000);
    }

    CAN0.setMode(MCP_NORMAL);

    Serial.println("Traffic Sign ECU Ready");
    Serial.println("Type Speed Limit and Press ENTER");
}

void loop()
{
    // Read serial input safely
    while (Serial.available())
    {
        char c = Serial.read();

        if (c == '\n')
        {
            speedLimit = inputString.toInt();

            Serial.print("New Speed Limit: ");
            Serial.println(speedLimit);

            inputString = "";
        }
        else
        {
            inputString += c;
        }
    }

    // Prepare CAN Data
    byte data[8] = {speedLimit, 0,0,0,0,0,0,0};

    // Send CAN Message
    byte result = CAN0.sendMsgBuf(0x100, 0, 8, data);

    if(result == CAN_OK)
    {
        Serial.print("Sent Speed Limit: ");
        Serial.println(speedLimit);
    }
    else
    {
        Serial.println("CAN Send Error");
    }

    delay(1000);
}