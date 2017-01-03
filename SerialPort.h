#ifndef _SerialPort_h_
#define _SerialPort_h_


#include <string>
#include <vector>
#include <stdexcept>
#include <termios.h>


class SerialPort
{
public:
    enum BaudRate {
        BAUD_50      = B50,
        BAUD_75      = B75,
        BAUD_110     = B110,
        BAUD_134     = B134,
        BAUD_150     = B150,
        BAUD_200     = B200,
        BAUD_300     = B300,
        BAUD_600     = B600,
        BAUD_1200    = B1200,
        BAUD_1800    = B1800,
        BAUD_2400    = B2400,
        BAUD_4800    = B4800,
        BAUD_9600    = B9600,
        BAUD_19200   = B19200,
        BAUD_38400   = B38400,
        BAUD_57600   = B57600,
        BAUD_115200  = B115200,
        BAUD_230400  = B230400,
        BAUD_460800  = B460800,
        BAUD_DEFAULT = B57600
    }; 

    enum CharacterSize {
        CHAR_SIZE_5  = CS5, 
        CHAR_SIZE_6  = CS6, 
        CHAR_SIZE_7  = CS7, 
        CHAR_SIZE_8  = CS8, 
        CHAR_SIZE_DEFAULT = CHAR_SIZE_8
    } ;

    enum StopBits {
        STOP_BITS_1,   
        STOP_BITS_2,   
        STOP_BITS_DEFAULT = STOP_BITS_1
    } ;

    enum Parity {
        PARITY_EVEN,     
        PARITY_ODD,      
        PARITY_NONE,     
        PARITY_DEFAULT = PARITY_NONE
    } ;

    enum FlowControl {
        FLOW_CONTROL_HARD,
        FLOW_CONTROL_SOFT,
        FLOW_CONTROL_NONE,
        FLOW_CONTROL_DEFAULT = FLOW_CONTROL_NONE
    } ;

    class NotOpen : public std::logic_error
    {
    public:
        NotOpen(const std::string& whatArg) :
            logic_error(whatArg) { }
    } ;

    class OpenFailed : public std::runtime_error
    {
    public:
        OpenFailed(const std::string& whatArg) :
            runtime_error(whatArg) { }
    } ;

    class AlreadyOpen : public std::logic_error
    {
    public:
        AlreadyOpen( const std::string& whatArg ) :
            logic_error(whatArg) { }
    } ;

    class UnsupportedBaudRate : public std::runtime_error
    {
    public:
        UnsupportedBaudRate( const std::string& whatArg ) :
            runtime_error(whatArg) { }
    } ;

    class ReadTimeout : public std::runtime_error
    {
    public:
        ReadTimeout() : runtime_error( "Read timeout" ) { }
    } ;

    explicit SerialPort( const std::string& serialPortName ) ;

    ~SerialPort() throw() ;

    void
    Open( const BaudRate      baudRate    = BAUD_DEFAULT,
          const CharacterSize charSize    = CHAR_SIZE_DEFAULT,
          const Parity        parityType  = PARITY_DEFAULT,
          const StopBits      stopBits    = STOP_BITS_DEFAULT,
          const FlowControl   flowControl = FLOW_CONTROL_DEFAULT )
        throw( AlreadyOpen,
               OpenFailed,
               UnsupportedBaudRate,
               std::invalid_argument ) ;

    bool
    IsOpen() const ;

    void
    Close()
        throw(NotOpen) ;

    void
    SetBaudRate( const BaudRate baudRate )
        throw( UnsupportedBaudRate,
               NotOpen,
               std::invalid_argument ) ;

    BaudRate
    GetBaudRate() const
        throw( NotOpen,
               std::runtime_error ) ;

    void
    SetCharSize( const CharacterSize charSize )
        throw( NotOpen,
               std::invalid_argument ) ;
    CharacterSize
    GetCharSize() const
        throw(NotOpen) ;

    void
    SetParity( const Parity parityType )
        throw( NotOpen,
               std::invalid_argument ) ;

    Parity
    GetParity() const
        throw(NotOpen) ;

    void
    SetNumOfStopBits( const StopBits numOfStopBits )
        throw( NotOpen,
               std::invalid_argument ) ;

    StopBits
    GetNumOfStopBits() const
        throw(NotOpen) ;

    void
    SetFlowControl( const FlowControl   flowControl )
        throw( NotOpen,
               std::invalid_argument ) ;

    FlowControl
    GetFlowControl() const
        throw( NotOpen ) ;

    bool
    IsDataAvailable() const
        throw(NotOpen) ;

    unsigned char
    ReadByte( const unsigned int msTimeout = 0 )
        throw( NotOpen,
               ReadTimeout,
               std::runtime_error ) ;

    typedef std::vector<unsigned char> DataBuffer ;
    void
    Read( DataBuffer&        dataBuffer,
          const unsigned int numOfBytes = 0,
          const unsigned int msTimeout  = 0 )
        throw( NotOpen,
               ReadTimeout,
               std::runtime_error ) ;


    const std::string
    ReadLine( const unsigned int msTimeout = 0,
              const char         lineTerminator = '\n' )
        throw( NotOpen,
               ReadTimeout,
               std::runtime_error ) ;

    void
    WriteByte(const unsigned char dataByte)
        throw( NotOpen,
               std::runtime_error ) ;

    void
    Write(const DataBuffer& dataBuffer)
        throw( NotOpen,
               std::runtime_error ) ;

    void
    Write(const std::string& dataString)
        throw( NotOpen,
               std::runtime_error ) ;
private:
    SerialPort( const SerialPort& otherSerialPort ) ;
    SerialPort& operator=(const SerialPort& otherSerialPort ) ;
    class SerialPortImpl ;
    SerialPortImpl* mSerialPortImpl ;
} ;

#endif // #ifndef _SerialPort_h