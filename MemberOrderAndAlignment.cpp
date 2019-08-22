#include <iostream>
#include <string>
#include <atomic>

#define DOCS "spikes on member order and alignment on template or inherited structs\n"           \
             "=====================================================================\n"           \
             "\n"                                                                                \
             "This program attempts to investigate how one can specify member order\n"           \
             "for a third struct based on two others. \n"                                        \
             "This is useful for template data structures, which often need to add\n"            \
             "some fields on a user provided struct -- and for false-sharing prevention\n"       \
             "purposes, some fields must be the first ones on the struct.\n"                     \
             "\n"                                                                                \
             "--> NOTE on the order of fields:\n"                                                \
             "      in the following tests, '->next' should be the first element of the\n"       \
             "      composed structure, in order to ensure optimal mem & cpu performance,\n"     \
             "      since '->next' has a 64 byte alignment.\n"                                   \
             "\n"                                                                                \
             "--> NOTE on composed struct size:\n"                                               \
             "      you should check if the instance size is more than what it needs to be --\n" \
             "      it is expected to be more just to honror the 64 byte alignment, but if it\n" \
             "      is greater than that, it means hidden fields were added to the struct,\n"    \
             "      which is totally unacceptable if one would ever try to mmap these structures.\n"

template <unsigned _padding>
struct UserData {
    unsigned userField;
    char     padding[_padding];
};

struct AdditionalFields {
//    alignas(64) std::atomic<unsigned>* next;
    std::atomic<unsigned>* next;
};


// composed structs
///////////////////

template <typename _UserStruct>
struct alignas(64) SimpleInheritance: public _UserStruct {
    std::atomic<unsigned>* next;
};

template <typename _UserStruct>
struct alignas(64) DoubleInheritance: public AdditionalFields , _UserStruct {};

template <typename _UserStruct>
struct ZeroInheritance {
    alignas(64) std::atomic<unsigned>* next;
    _UserStruct                        data;
};


// spike functions
//////////////////

template <typename _AdditionalFields, typename _UserData>
void printStructInfo(_AdditionalFields& additionalFieldsInstance, _UserData& userDataInstance, std::string typeName) {
    _AdditionalFields arr[2];
    std::cout << "Testing '"<<typeName<<"':\n";
    std::cout << "\tAbout the alignment:\n"
                 "\t\t" << alignof(_AdditionalFields) <<                                                    ": alignof("<<typeName<<")\n"
                 "\t\t" << (reinterpret_cast<intptr_t>(&(arr[1]))-reinterpret_cast<intptr_t>(&(arr[0]))) << ": Array slot alignment\n";
    ;
    std::cout << "\tAbout the order of fields:\n"
                 "\t\taddr of a sample instance:         " << static_cast<void*>(&(additionalFieldsInstance)) << "\n"
                 "\t\tthe 'next' field address:          " << static_cast<void*>(&(additionalFieldsInstance.next)) << "\n"
                 "\t\tthe first field (userField) is at: " << static_cast<void*>(&(userDataInstance.userField)) << "\n"
                 "\t\t--> The order of fields is:        " << (static_cast<void*>(&(additionalFieldsInstance)) == static_cast<void*>(&(additionalFieldsInstance.next)) ? "->next" : typeName) << " first\n"
    ;
    std::cout << "\tAbout the size of '"<<typeName<<"':\n"
                 "\t\t" << sizeof(_AdditionalFields)             << " is the sizeof("<<typeName<<")\n"
                 "\t\t" << sizeof(additionalFieldsInstance)      << " is the sizeof("<<typeName<<" instance)\n"
                 "\t\t" << (sizeof(_AdditionalFields::next) +
                            sizeof(_UserData::userField)    +
                            sizeof(_UserData::padding))          << " is the sizeof(all known fields)\n"
                 "\t\t" << "   "                                 << sizeof(_AdditionalFields::next) << "+" << sizeof(_UserData::userField) << "+" << sizeof(_UserData::padding) << "\n\n"
    ;
}


int main(void) {

    std::cout << DOCS << "\n";

    constexpr unsigned expectedSizeOfComposedStructure = sizeof(std::atomic<unsigned>*)+sizeof(unsigned);

    std::cout << "*** The expected size of the composed structure is " << expectedSizeOfComposedStructure << "\n\n";

    typedef ZeroInheritance<UserData<60-expectedSizeOfComposedStructure>> ZeroInheritanceSize60;
    ZeroInheritanceSize60 zeroInheritanceSize60;
    printStructInfo(zeroInheritanceSize60, zeroInheritanceSize60.data, "ZeroInheritanceSize60");

    typedef SimpleInheritance<UserData<60-expectedSizeOfComposedStructure>> SimpleInheritanceSize60;
    SimpleInheritanceSize60 simpleInheritanceSize60;
    printStructInfo(simpleInheritanceSize60, simpleInheritanceSize60, "SimpleInheritanceSize60");

    typedef DoubleInheritance<UserData<60-expectedSizeOfComposedStructure>> DoubleInheritanceSize60;
    DoubleInheritanceSize60 doubleInheritanceSize60;
    printStructInfo(doubleInheritanceSize60, doubleInheritanceSize60, "DoubleInheritanceSize60");

    std::cout << "--> Conclusions:\n"
                 "      - 'SimpleInheritance' is not suitable, since ->next isn't at the beginning of the struct;\n"
                 "      - both 'DoubleInheritance' and 'ZeroInheritance' (composition) adhere to the requisites;\n"
                 "      - 'DoubleInheritance' is, possibly, more elegant, since the 'data' fields are accessed\n"
                 "        directly.\n\n"
                 "--> more info: https://stackoverflow.com/questions/2006504/c-data-alignment-member-order-inheritance\n\n";
    ;

}