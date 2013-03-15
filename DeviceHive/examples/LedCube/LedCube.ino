#include <Rainbowduino.h>
#include <DeviceHive.h>

#define CMD_STATUS_SUCCESS      "Success"
#define CMD_STATUS_FAILED       "Failed"
#define CMD_RESULT_OK           "OK"

// device registration data
// intent numbers should be greater than 255!
// please refer to http://www.devicehive.com/binary/#SystemMessages/RegisterJson for complete syntax of registration info
const char *REG_DATA = "{"
    "id:'b125698d-61bd-40d7-b65e-e1f86852a166',"
    "key:'LED_Cube',"
    "name:'LED Cube',"
    "deviceClass:{"
        "name:'LED_cube',"
        "version:'1.0'},"
    "equipment:[{code:'cube',name:'cube',type:'LED_Cube'}],"
    "commands:["
        "{intent:1001,name:'fill',params:{R:u8,G:u8,B:u8}},"
        "{intent:1002,name:'cube',params:[{R:u8,G:u8,B:u8}]},"
        "{intent:1003,name:'pixels',params:[{X:u8,Y:u8,Z:u8,R:u8,G:u8,B:u8}]}"
    "],"
    "notifications:[]"
"}";


struct Point        // point's coordinates
{
    uint8_t X;
    uint8_t Y;
    uint8_t Z;
};

struct Color        // RGB color components
{
    uint8_t R;
    uint8_t G;
    uint8_t B;
};

struct Pixel        // point + color
{
    Point point;
    Color color;
};

enum CubeSize       // Cube dimension
{
    NX = 4,
    NY = 4,
    NZ = 4
};


//char Point_size_check[(sizeof(Point)==3) ? 1 : -1];
//char Color_size_check[(sizeof(Color)==3) ? 1 : -1];
//char Pixel_size_check[(sizeof(Pixel)==6) ? 1 : -1];
InputMessage rx_msg; // received message


/**
 * Initializes the Arduino firmware.
 */
void setup(void)
{
    Rb.init(); // initialize Rainbowduino driver
    Rb.blankDisplay();

    Serial.begin(115200);
    DH.begin(Serial);
    DH.writeRegistrationResponse(REG_DATA);
}


/**
 * Loop procedure is called continuously.
 */
void loop(void)
{
    if (DH.read(rx_msg) == DH_PARSE_OK)
    {
        switch (rx_msg.intent)
        {
            case INTENT_REGISTRATION_REQUEST:   // registration data needed
                DH.writeRegistrationResponse(REG_DATA);
                break;


            case 1001:   // "fill" - fill the whole Cube with one color
            {
                const uint32_t cmd_id = rx_msg.getUInt32();
                Color color = rx_msg.get<Color>();

                for (int x = 0; x < NX; ++x)
                    for (int y = 0; y < NY; ++y)
                        for (int z = 0; z < NZ; ++z)
                {
                    Rb.setPixelZXY(z, x, y,
                        color.R, color.G, color.B);
                }

                DH.writeCommandResult(cmd_id, CMD_STATUS_SUCCESS, CMD_RESULT_OK);
            } break;


            case 1002:   // "cube" - fill the whole Cube with individual color for each pixel
            {
                const uint32_t cmd_id = rx_msg.getUInt32();
                const uint16_t count = rx_msg.getUInt16();

                if (count == NX*NY*NZ)
                {
                    for (int x = 0; x < NX; ++x)
                        for (int y = 0; y < NY; ++y)
                            for (int z = 0; z < NZ; ++z)
                    {
                        Color color = rx_msg.get<Color>();
                        Rb.setPixelZXY(z, x, y,
                            color.R, color.G, color.B);
                    }

                    DH.writeCommandResult(cmd_id, CMD_STATUS_SUCCESS, CMD_RESULT_OK);
                }
                else
                {
                    DH.writeCommandResult(cmd_id, CMD_STATUS_FAILED, "Invalid number of colors");
                }
            } break;


            case 1003: // "pixels" - fill several pixels
            {
                const uint32_t cmd_id = rx_msg.getUInt32();
                const uint16_t count = rx_msg.getUInt16();
                for (uint16_t i = 0; i < count; ++i)
                {
                    Pixel px = rx_msg.get<Pixel>();
                    Rb.setPixelZXY(px.point.Z, px.point.X, px.point.Y,
                                   px.color.R, px.color.G, px.color.B);
                }

                DH.writeCommandResult(cmd_id, CMD_STATUS_SUCCESS, CMD_RESULT_OK);
            } break;
        }

        rx_msg.reset(); // reset for the next message parsing
    }
}
