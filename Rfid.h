#include <string>
using namespace std;

/**
 * Class for accessing RFID Reader
*/
class Rfid {
    public:
        
        /**
         * Write data to the RFID Chip
        */
        void writeData(string* hashData);

        /**
         * Read data from the RFID Chip
        */
        string* readData();

    private:

        /**
         * Access Rfid Chip
         * accessMode 0 is reading, 1 is writing
        */
        bool accessRfidChip(string* hashData = NULL, int accessMode=0);
};