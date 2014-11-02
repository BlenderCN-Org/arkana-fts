
namespace FTSSrv2 {

class ConnectionWaiter {
public:
    virtual ~ConnectionWaiter() {};

    virtual int init(uint16_t in_usPort) = 0;
    virtual bool waitForThenDoConnection(uint64_t in_ulMaxWaitMillisec = FTSC_TIME_OUT) = 0;
    virtual int deinit() = 0;

protected:
    ConnectionWaiter() {};
};

} // namespace FTSSrv2