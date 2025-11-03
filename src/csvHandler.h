#ifndef CSV_HANDLER
#define CSV_HANDLER

#include <string>

class CSVHandler {
    public:
        std::string generateRowData(const int move);
        void exportMove(const int move);
};

#endif