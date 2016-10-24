
#include "AdsLibOOI/AdsLibOOI.h"
#include "AdsLibOOI/AdsDevice.h"

#include <iostream>
#include <iomanip>

#include <fructose/fructose.h>
using namespace fructose;

static const AmsNetId serverNetId {192, 168, 0, 231, 1, 1};
static const AmsAddr server {serverNetId, AMSPORT_R0_PLC_TC3};
static const AmsAddr serverBadPort {serverNetId, 1000};

static size_t g_NumNotifications = 0;
static void NotifyCallback(const AmsAddr* pAddr, const AdsNotificationHeader* pNotification, uint32_t hUser)
{
    ++g_NumNotifications;
#if 0
    std::cout << std::setfill('0') <<
        "hUser 0x" << std::hex << std::setw(4) << hUser <<
        " sample time: " << std::dec << pNotification->nTimeStamp <<
        " sample size: " << std::dec << pNotification->cbSampleSize <<
        " value: 0x" << std::hex << (int)pNotification->data[0] << '\n';
#endif
}

void print(const AmsAddr& addr, std::ostream& out)
{
    out << "AmsAddr: " << std::dec <<
    (int)addr.netId.b[0] << '.' << (int)addr.netId.b[1] << '.' << (int)addr.netId.b[2] << '.' <<
    (int)addr.netId.b[3] << '.' << (int)addr.netId.b[4] << '.' << (int)addr.netId.b[5] << ':' <<
        addr.port << '\n';
}

long testPortOpen(std::ostream& out)
{
    long port = AdsPortOpenEx();
    if (!port) {
        return 0;
    }

    AmsAddr addr;
    if (AdsGetLocalAddressEx(port, &addr)) {
        AdsPortCloseEx(port);
        return 0;
    }
    out << "Port: " << port << ' ';
    print(addr, out);
    return port;
}

struct TestAds : test_base<TestAds> {
    static const int NUM_TEST_LOOPS = 10;
    std::ostream& out;

    TestAds(std::ostream& outstream)
        : out(outstream)
    {}

    void testAdsPortOpenEx(const std::string&)
    {
        static const size_t NUM_TEST_PORTS = 2;
        std::vector<AdsRoute> routes;

        for (size_t i = 0; i < NUM_TEST_PORTS; ++i) {
            routes.emplace_back("192.168.0.232", serverNetId, AMSPORT_R0_PLC_TC3, AMSPORT_R0_PLC_TC3);
            fructose_loop_assert(i, 0 != routes.back()->GetLocalPort());
        }
    }

    void testAdsReadReqEx2(const std::string&)
    {
        {
            AdsRoute route {"192.168.0.232", serverNetId, AMSPORT_R0_PLC_TC3, AMSPORT_R0_PLC_TC3};
            fructose_assert(0 != route->GetLocalPort());

            print(route->GetSymbolsAmsAddr(), out);

            AdsVariable<uint32_t> buffer {route, 0x4020, 0};
            for (int i = 0; i < NUM_TEST_LOOPS; ++i) {
                fructose_loop_assert(i, 0 == buffer);
            }
        }

        // provide out of range port
        /* not possible with OOI */

        // provide nullptr to AmsAddr
        /* not possible with OOI */

        // provide unknown AmsAddr
        try {
            AdsRoute unknownAmsAddrRoute {"192.168.0.232", {1, 2, 3, 4, 5, 6}, AMSPORT_R0_PLC_TC3, AMSPORT_R0_PLC_TC3};
            AdsVariable<uint32_t> buffer {unknownAmsAddrRoute, 0x4020, 0};
            fructose_assert(0 == buffer);
            fructose_assert(false);
        } catch (AdsException ex) {
            fructose_assert(GLOBALERR_MISSING_ROUTE == ex.getErrorCode());
        }

        // provide nullptr to bytesRead
        /* not possible with OOI */

        // provide nullptr to buffer
        /* not possible with OOI */

        // provide 0 length buffer
        /* not possible with OOI */

        // provide invalid indexGroup
        try {
            AdsRoute route {"192.168.0.232", serverNetId, AMSPORT_R0_PLC_TC3, AMSPORT_R0_PLC_TC3};
            AdsVariable<uint32_t> buffer {route, 0, 0};
            fructose_assert(0 == buffer);
            fructose_assert(false);
        } catch (AdsException ex) {
            fructose_assert(ADSERR_DEVICE_SRVNOTSUPP == ex.getErrorCode());
        }

        // provide invalid indexOffset
        try {
            AdsRoute route {"192.168.0.232", serverNetId, AMSPORT_R0_PLC_TC3, AMSPORT_R0_PLC_TC3};
            AdsVariable<uint32_t> buffer {route, 0x4025, 0x10000};
            fructose_assert(0 == buffer);
            fructose_assert(false);
        } catch (AdsException ex) {
            fructose_assert(ADSERR_DEVICE_SRVNOTSUPP == ex.getErrorCode());
        }
    }

    void testAdsReadDeviceInfoReqEx(const std::string&)
    {
        static const char NAME[] = "Plc30 App";
        {
            AdsRoute route {"192.168.0.232", serverNetId, AMSPORT_R0_PLC_TC3, AMSPORT_R0_PLC_TC3};
            fructose_assert(0 != route->GetLocalPort());

            AdsDevice device {route};
            for (int i = 0; i < NUM_TEST_LOOPS; ++i) {
                fructose_loop_assert(i, 3 == device.m_Info.version.version);
                fructose_loop_assert(i, 1 == device.m_Info.version.revision);
                fructose_loop_assert(i, 1202 == device.m_Info.version.build);
                fructose_loop_assert(i, 0 == strncmp(device.m_Info.name, NAME, sizeof(NAME)));
            }
        }

        // provide out of range port
        /* not possible with OOI */

        // provide nullptr to AmsAddr
        /* not possible with OOI */

        // provide unknown AmsAddr
        try {
            AdsRoute unknownAmsAddrRoute {"192.168.0.232", {1, 2, 3, 4, 5, 6}, AMSPORT_R0_PLC_TC3, AMSPORT_R0_PLC_TC3};
            AdsDevice device {unknownAmsAddrRoute};
            fructose_assert(0 == device.m_Info.version.version);
            fructose_assert(false);
        } catch (AdsException ex) {
            fructose_assert(GLOBALERR_MISSING_ROUTE == ex.getErrorCode());
        }

        // provide nullptr to devName/version
        /* not possible with OOI */
    }

    void testAdsReadStateReqEx(const std::string&)
    {
        {
            AdsRoute route {"192.168.0.232", serverNetId, AMSPORT_R0_PLC_TC3, AMSPORT_R0_PLC_TC3};
            fructose_assert(0 != route->GetLocalPort());

            AdsDevice device {route};
            const auto state = device.GetState();
            fructose_assert(ADSSTATE_RUN == state.ads);
            fructose_assert(0 == state.device);
        }

        // provide bad server port
        try {
            AdsRoute badAmsAddrRoute {"192.168.0.232", serverNetId, 1000, 1000};
            AdsDevice device {badAmsAddrRoute};
            const auto state = device.GetState();
            fructose_assert(0 == state.device);
            fructose_assert(false);
        } catch (AdsException ex) {
            fructose_assert(GLOBALERR_TARGET_PORT == ex.getErrorCode());
        }

        // provide out of range port
        /* not possible with OOI */

        // provide nullptr to AmsAddr
        /* not possible with OOI */

        // provide unknown AmsAddr
        try {
            AdsRoute unknownAmsAddrRoute {"192.168.0.232", {1, 2, 3, 4, 5, 6}, AMSPORT_R0_PLC_TC3, AMSPORT_R0_PLC_TC3};
            AdsDevice device {unknownAmsAddrRoute};
            const auto state = device.GetState();
            fructose_assert(0 == state.device);
            fructose_assert(false);
        } catch (AdsException ex) {
            fructose_assert(GLOBALERR_MISSING_ROUTE == ex.getErrorCode());
        }

        // provide nullptr to adsState/devState
        /* not possible with OOI */
    }

    void testAdsReadWriteReqEx2(const std::string&)
    {
        static const char handleName[] = "MAIN.byByte";
        {
            AdsRoute route {"192.168.0.232", serverNetId, AMSPORT_R0_PLC_TC3, AMSPORT_R0_PLC_TC3};
            fructose_assert(0 != route->GetLocalPort());

            print(route->GetSymbolsAmsAddr(), out);

            uint32_t outBuffer = 0xDEADBEEF;
            AdsVariable<uint32_t> buffer {route, handleName};
            for (int i = 0; i < NUM_TEST_LOOPS; ++i) {
                buffer = outBuffer;
                fructose_loop_assert(i, outBuffer == buffer);
                outBuffer = ~outBuffer;
            }
            buffer = 0x0; /* restore default value */
        }

        // provide out of range port
        /* not possible with OOI */

        // provide nullptr to AmsAddr
        /* not possible with OOI */

        // provide unknown AmsAddr
        try {
            AdsRoute unknownAmsAddrRoute {"192.168.0.232", {1, 2, 3, 4, 5, 6}, AMSPORT_R0_PLC_TC3, AMSPORT_R0_PLC_TC3};
            AdsVariable<uint32_t> buffer {unknownAmsAddrRoute, handleName};
            fructose_assert(0 == buffer);
            fructose_assert(false);
        } catch (AdsException ex) {
            fructose_assert(GLOBALERR_MISSING_ROUTE == ex.getErrorCode());
        }

        // provide nullptr to bytesRead
        /* not possible with OOI */

        // provide nullptr to readBuffer
        /* not possible with OOI */

        // provide 0 length readBuffer
        /* not possible with OOI */

        // provide nullptr to writeBuffer
        /* not possible with OOI */

        // provide 0 length writeBuffer
        /* not possible with OOI */

        // provide invalid symbolName
        try {
            AdsRoute route {"192.168.0.232", serverNetId, AMSPORT_R0_PLC_TC3, AMSPORT_R0_PLC_TC3};
            AdsVariable<uint32_t> buffer {route, "xxx"};
            fructose_assert(0 == buffer);
            fructose_assert(false);
        } catch (AdsException ex) {
            fructose_assert(ADSERR_DEVICE_SYMBOLNOTFOUND == ex.getErrorCode());
        }

        // provide invalid indexGroup
        try {
            AdsRoute route {"192.168.0.232", serverNetId, AMSPORT_R0_PLC_TC3, AMSPORT_R0_PLC_TC3};
            AdsVariable<uint32_t> buffer {route, 0, 0};
            fructose_assert(0 == buffer);
            fructose_assert(false);
        } catch (AdsException ex) {
            fructose_assert(ADSERR_DEVICE_SRVNOTSUPP == ex.getErrorCode());
        }

        // provide invalid indexOffset
        try {
            AdsRoute route {"192.168.0.232", serverNetId, AMSPORT_R0_PLC_TC3, AMSPORT_R0_PLC_TC3};
            AdsVariable<uint32_t> buffer {route, 0x4025, 0x10000};
            fructose_assert(0 == buffer);
            fructose_assert(false);
        } catch (AdsException ex) {
            fructose_assert(ADSERR_DEVICE_SRVNOTSUPP == ex.getErrorCode());
        }
    }

    void testAdsWriteReqEx(const std::string&)
    {
        {
            AdsRoute route {"192.168.0.232", serverNetId, AMSPORT_R0_PLC_TC3, AMSPORT_R0_PLC_TC3};
            fructose_assert(0 != route->GetLocalPort());

            print(route->GetSymbolsAmsAddr(), out);

            uint32_t outBuffer = 0xDEADBEEF;
            AdsVariable<uint32_t> buffer {route, 0x4020, 0};
            for (int i = 0; i < NUM_TEST_LOOPS; ++i) {
                buffer = outBuffer;
                fructose_loop_assert(i, outBuffer == buffer);
                outBuffer = ~outBuffer;
            }
            buffer = 0x0; /* restore default value */
        }

        // provide out of range port
        /* not possible with OOI */

        // provide nullptr to AmsAddr
        /* not possible with OOI */

        // provide unknown AmsAddr
        try {
            AdsRoute unknownAmsAddrRoute {"192.168.0.232", {1, 2, 3, 4, 5, 6}, AMSPORT_R0_PLC_TC3, AMSPORT_R0_PLC_TC3};
            AdsVariable<uint32_t> buffer {unknownAmsAddrRoute, 0x4020, 0};
            buffer = 0;
            fructose_assert(false);
        } catch (AdsException ex) {
            fructose_assert(GLOBALERR_MISSING_ROUTE == ex.getErrorCode());
        }

        // provide nullptr to writeBuffer
        /* not possible with OOI */

        // provide 0 length writeBuffer
        /* not possible with OOI */

        // provide invalid symbolName
        try {
            AdsRoute route {"192.168.0.232", serverNetId, AMSPORT_R0_PLC_TC3, AMSPORT_R0_PLC_TC3};
            AdsVariable<uint32_t> buffer {route, "xxx"};
            fructose_assert(0 == buffer);
            fructose_assert(false);
        } catch (AdsException ex) {
            fructose_assert(ADSERR_DEVICE_SYMBOLNOTFOUND == ex.getErrorCode());
        }

        // provide invalid indexGroup
        try {
            AdsRoute route {"192.168.0.232", serverNetId, AMSPORT_R0_PLC_TC3, AMSPORT_R0_PLC_TC3};
            AdsVariable<uint32_t> buffer {route, 0, 0};
            fructose_assert(0 == buffer);
            fructose_assert(false);
        } catch (AdsException ex) {
            fructose_assert(ADSERR_DEVICE_SRVNOTSUPP == ex.getErrorCode());
        }
    }

    void testAdsWriteControlReqEx(const std::string&)
    {
        AdsRoute route {"192.168.0.232", serverNetId, AMSPORT_R0_PLC_TC3, AMSPORT_R0_PLC_TC3};
        fructose_assert(0 != route->GetLocalPort());

        AdsDevice device {route};
        for (int i = 0; i < NUM_TEST_LOOPS; ++i) {
            device.SetState(ADSSTATE_STOP, ADSSTATE_INVALID);
            auto state = device.GetState();
            fructose_loop_assert(i, ADSSTATE_STOP == state.ads);
            fructose_loop_assert(i, ADSSTATE_INVALID == state.device);
            device.SetState(ADSSTATE_RUN, ADSSTATE_INVALID);
            state = device.GetState();
            fructose_loop_assert(i, ADSSTATE_RUN == state.ads);
            fructose_loop_assert(i, ADSSTATE_INVALID == state.device);
        }

        // provide out of range port
        /* not possible with OOI */

        // provide nullptr to AmsAddr
        /* not possible with OOI */

        // provide unknown AmsAddr
        try {
            AdsRoute unknownAmsAddrRoute {"192.168.0.232", {1, 2, 3, 4, 5, 6}, AMSPORT_R0_PLC_TC3, AMSPORT_R0_PLC_TC3};
            AdsDevice device {unknownAmsAddrRoute};
            device.SetState(ADSSTATE_STOP, ADSSTATE_INVALID);
            fructose_assert(false);
        } catch (AdsException ex) {
            fructose_assert(GLOBALERR_MISSING_ROUTE == ex.getErrorCode());
        }

        // provide invalid adsState
        try {
            device.SetState(ADSSTATE_INVALID, ADSSTATE_INVALID);
            fructose_assert(false);
        } catch (AdsException ex) {
            fructose_assert(ADSERR_DEVICE_SRVNOTSUPP == ex.getErrorCode());
        }
        try {
            device.SetState(ADSSTATE_MAXSTATES, ADSSTATE_INVALID);
            fructose_assert(false);
        } catch (AdsException ex) {
            fructose_assert(ADSERR_DEVICE_SRVNOTSUPP == ex.getErrorCode());
        }

        // provide invalid devState
        // TODO find correct parameters for this test

        // provide trash buffer
        /* not possible with OOI */
    }

    void testAdsNotification(const std::string&)
    {
        const long port = AdsPortOpenEx();

        fructose_assert(0 != port);

        static const size_t MAX_NOTIFICATIONS_PER_PORT = 1024;
        static const size_t LEAKED_NOTIFICATIONS = MAX_NOTIFICATIONS_PER_PORT / 2;
        uint32_t notification[MAX_NOTIFICATIONS_PER_PORT];
        AdsNotificationAttrib attrib = { 1, ADSTRANS_SERVERCYCLE, 0, {1000000} };
        uint32_t hUser = 0xDEADBEEF;

        // provide out of range port
        fructose_assert(ADSERR_CLIENT_PORTNOTOPEN ==
                        AdsSyncAddDeviceNotificationReqEx(0, &server, 0x4020, 0, &attrib, &NotifyCallback, hUser,
                                                          &notification[0]));

        // provide nullptr to AmsAddr
        fructose_assert(ADSERR_CLIENT_NOAMSADDR ==
                        AdsSyncAddDeviceNotificationReqEx(port, nullptr, 0x4020, 0, &attrib, &NotifyCallback, hUser,
                                                          &notification[0]));

        // provide unknown AmsAddr
        AmsAddr unknown { { 1, 2, 3, 4, 5, 6 }, AMSPORT_R0_PLC_TC3 };
        fructose_assert(GLOBALERR_MISSING_ROUTE ==
                        AdsSyncAddDeviceNotificationReqEx(port, &unknown, 0x4020, 0, &attrib, &NotifyCallback, hUser,
                                                          &notification[0]));

        // provide invalid indexGroup
        fructose_assert(ADSERR_DEVICE_SRVNOTSUPP ==
                        AdsSyncAddDeviceNotificationReqEx(port, &server, 0, 0, &attrib, &NotifyCallback, hUser,
                                                          &notification[0]));

        // provide invalid indexOffset
        fructose_assert(ADSERR_DEVICE_SRVNOTSUPP ==
                        AdsSyncAddDeviceNotificationReqEx(port, &server, 0x4025, 0x10000, &attrib, &NotifyCallback,
                                                          hUser,
                                                          &notification[0]));

        // provide nullptr to attrib/callback/hNotification
        fructose_assert(ADSERR_CLIENT_INVALIDPARM ==
                        AdsSyncAddDeviceNotificationReqEx(port, &server, 0x4020, 4, nullptr, &NotifyCallback, hUser,
                                                          &notification[0]));
        fructose_assert(ADSERR_CLIENT_INVALIDPARM ==
                        AdsSyncAddDeviceNotificationReqEx(port, &server, 0x4020, 4, &attrib, nullptr, hUser,
                                                          &notification[0]));
        fructose_assert(ADSERR_CLIENT_INVALIDPARM ==
                        AdsSyncAddDeviceNotificationReqEx(port, &server, 0x4020, 4, &attrib, &NotifyCallback, hUser,
                                                          nullptr));

        // delete nonexisting notification
        fructose_assert(ADSERR_CLIENT_REMOVEHASH == AdsSyncDelDeviceNotificationReqEx(port, &server, 0xDEADBEEF));

        // normal test
        for (hUser = 0; hUser < MAX_NOTIFICATIONS_PER_PORT; ++hUser) {
            fructose_loop_assert(hUser,
                                 0 ==
                                 AdsSyncAddDeviceNotificationReqEx(port, &server, 0x4020, 4, &attrib, &NotifyCallback,
                                                                   hUser,
                                                                   &notification[hUser]));
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        for (hUser = 0; hUser < MAX_NOTIFICATIONS_PER_PORT - LEAKED_NOTIFICATIONS; ++hUser) {
            fructose_loop_assert(hUser, 0 == AdsSyncDelDeviceNotificationReqEx(port, &server, notification[hUser]));
        }
        fructose_assert(0 == AdsPortCloseEx(port));
    }

    void testAdsTimeout(const std::string&)
    {
        const long port = AdsPortOpenEx();
        uint32_t timeout;

        fructose_assert(0 != port);
        fructose_assert(ADSERR_CLIENT_PORTNOTOPEN == AdsSyncGetTimeoutEx(55555, &timeout));
        fructose_assert(0 == AdsSyncGetTimeoutEx(port, &timeout));
        fructose_assert(5000 == timeout);
        fructose_assert(0 == AdsSyncSetTimeoutEx(port, 1000));
        fructose_assert(0 == AdsSyncGetTimeoutEx(port, &timeout));
        fructose_assert(1000 == timeout);
        fructose_assert(0 == AdsSyncSetTimeoutEx(port, 5000));

        timeout = 0;
        // provide out of range port
        fructose_assert(ADSERR_CLIENT_PORTNOTOPEN == AdsSyncGetTimeoutEx(0, &timeout));
        fructose_assert(ADSERR_CLIENT_PORTNOTOPEN == AdsSyncSetTimeoutEx(0, 2000));
        fructose_assert(0 == timeout);

        // provide nullptr to timeout
        fructose_assert(ADSERR_CLIENT_INVALIDPARM == AdsSyncGetTimeoutEx(port, nullptr));
        fructose_assert(0 == AdsPortCloseEx(port));
    }
};

struct TestAdsPerformance : test_base<TestAdsPerformance> {
    std::ostream& out;
    bool runEndurance;

    TestAdsPerformance(std::ostream& outstream)
        : out(outstream),
        runEndurance(false)
    {
        AdsAddRoute(serverNetId, "192.168.0.232");
    }
#ifdef WIN32
    ~TestAdsPerformance()
    {
        // WORKAROUND: On Win7-64 AdsConnection::~AdsConnection() is triggered by the destruction
        //             of the static AdsRouter object and hangs in receive.join()
        AdsDelRoute(serverNetId);
    }
#endif

    void testLargeFrames(const std::string&)
    {
        // TODO testLargeFrames
        fructose_assert(false);
    }

    void testManyNotifications(const std::string& testname)
    {
        std::thread threads[8];
        g_NumNotifications = 0;
        const auto start = std::chrono::high_resolution_clock::now();
        for (auto& t : threads) {
            t = std::thread(&TestAdsPerformance::Notifications, this, 1024);
        }
        for (auto& t : threads) {
            t.join();
        }
        const auto end = std::chrono::high_resolution_clock::now();
        const auto tmms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        out << testname << ' ' << g_NumNotifications / tmms << " notifications/ms (" << g_NumNotifications << '/' <<
            tmms << ")\n";
    }

    void testParallelReadAndWrite(const std::string& testname)
    {
        std::thread threads[96];
        const auto start = std::chrono::high_resolution_clock::now();
        for (auto& t : threads) {
            t = std::thread(&TestAdsPerformance::Read, this, 1024);
        }
        for (auto& t : threads) {
            t.join();
        }
        const auto end = std::chrono::high_resolution_clock::now();
        const auto tmms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        out << testname << " took " << tmms << "ms\n";
    }

    void testEndurance(const std::string& testname)
    {
        static const size_t numNotifications = 1024;
        const long port = AdsPortOpenEx();
        fructose_assert(0 != port);

        const auto notification = std::unique_ptr<uint32_t[]>(new uint32_t[numNotifications]);
        AdsNotificationAttrib attrib = { 1, ADSTRANS_SERVERCYCLE, 0, {1000000} };
        uint32_t hUser = 0xDEADBEEF;

        runEndurance = true;
        std::thread threads[1];
        for (auto& t : threads) {
            t = std::thread(&TestAdsPerformance::Read, this, 1024);
        }

        const auto start = std::chrono::high_resolution_clock::now();
        for (hUser = 0; hUser < numNotifications; ++hUser) {
            fructose_assert_eq(0,
                               AdsSyncAddDeviceNotificationReqEx(port, &server, 0x4020, 4, &attrib, &NotifyCallback,
                                                                 hUser,
                                                                 &notification[hUser]));
        }

        std::cout << "Hit ENTER to stop endurance test\n";
        std::cin.ignore();
        runEndurance = false;

        for (hUser = 0; hUser < numNotifications; ++hUser) {
            fructose_assert_eq(0, AdsSyncDelDeviceNotificationReqEx(port, &server, notification[hUser]));
        }
        const auto end = std::chrono::high_resolution_clock::now();
        const auto tmms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        for (auto& t : threads) {
            t.join();
        }
        out << testname << ' ' << 1000 * g_NumNotifications / tmms << " notifications/s (" << g_NumNotifications <<
            '/' << tmms << ")\n";
    }

private:
    void Notifications(size_t numNotifications)
    {
        const long port = AdsPortOpenEx();
        fructose_assert(0 != port);

        const auto notification = std::unique_ptr<uint32_t[]>(new uint32_t[numNotifications]);
        AdsNotificationAttrib attrib = { 1, ADSTRANS_SERVERCYCLE, 0, {1000000} };
        uint32_t hUser = 0xDEADBEEF;

        for (hUser = 0; hUser < numNotifications; ++hUser) {
            fructose_assert_eq(0,
                               AdsSyncAddDeviceNotificationReqEx(port, &server, 0x4020, 4, &attrib, &NotifyCallback,
                                                                 hUser,
                                                                 &notification[hUser]));
        }
        std::this_thread::sleep_for(std::chrono::seconds(5));
        for (hUser = 0; hUser < numNotifications; ++hUser) {
            fructose_assert_eq(0, AdsSyncDelDeviceNotificationReqEx(port, &server, notification[hUser]));
        }
        fructose_assert(0 == AdsPortCloseEx(port));
    }

    void Read(const size_t numLoops)
    {
        const long port = AdsPortOpenEx();
        fructose_assert(0 != port);

        uint32_t bytesRead;
        uint32_t buffer;
        do {
            for (size_t i = 0; i < numLoops; ++i) {
                fructose_loop_assert(i,
                                     0 ==
                                     AdsSyncReadReqEx2(port, &server, 0x4020, 0, sizeof(buffer), &buffer, &bytesRead));
                fructose_loop_assert(i, sizeof(buffer) == bytesRead);
                fructose_loop_assert(i, 0 == buffer);
            }
        } while (runEndurance);
        fructose_assert(0 == AdsPortCloseEx(port));
    }
};

int main()
{
#if 0
    std::ostream nowhere(0);
    std::ostream& errorstream = nowhere;
#else
    std::ostream& errorstream = std::cout;
#endif
    TestAds adsTest(errorstream);
    adsTest.add_test("testAdsPortOpenEx", &TestAds::testAdsPortOpenEx);
    adsTest.add_test("testAdsReadReqEx2", &TestAds::testAdsReadReqEx2);
    adsTest.add_test("testAdsReadDeviceInfoReqEx", &TestAds::testAdsReadDeviceInfoReqEx);
    adsTest.add_test("testAdsReadStateReqEx", &TestAds::testAdsReadStateReqEx);
    adsTest.add_test("testAdsReadWriteReqEx2", &TestAds::testAdsReadWriteReqEx2);
    adsTest.add_test("testAdsWriteReqEx", &TestAds::testAdsWriteReqEx);
    adsTest.add_test("testAdsWriteControlReqEx", &TestAds::testAdsWriteControlReqEx);
#if 0
    adsTest.add_test("testAdsNotification", &TestAds::testAdsNotification);
    adsTest.add_test("testAdsTimeout", &TestAds::testAdsTimeout);
#endif
    adsTest.run();

    TestAdsPerformance performance(errorstream);
    performance.add_test("testManyNotifications", &TestAdsPerformance::testManyNotifications);
    performance.add_test("testParallelReadAndWrite", &TestAdsPerformance::testParallelReadAndWrite);
//	performance.add_test("testEndurance", &TestAdsPerformance::testEndurance);
    //performance.run();

    std::cout << "Hit ENTER to continue\n";
    std::cin.ignore();
}