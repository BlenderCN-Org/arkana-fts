#include "dLib/aTest/TestHarness.h"
#include "dLib/dString/dPath.h"
#include "dLib/dFile/dFile.h"
#include "dLib/dString/dString.h"
#include "logging/ftslogger.h"
#include "scripting/DaoVm.h"
#include "input/input.h"
#include "sound/fts_Snd.h" // To init the sound system.
#include "dao.h"

#include "scripting/DaoFunction.h"
#include <tuple>
#include <type_traits>
#include <list>

using namespace FTS;

extern "C" {
extern DaoVmSpace   *mainVmSpace ;
extern DaoVmProcess *mainVmProcess ;
}
class SuiteDaoVmSetup : public TestSetup {
public:
    void setup()
    {
        (new DefaultLogger())->stfu();
        new InputManager();
        new DaoVm();
    }
    void teardown()
	{
		delete DaoVm::getSingletonPtr();
        mainVmSpace = nullptr;
        mainVmProcess = nullptr;
        delete DefaultLogger::getSingletonPtr();
        delete InputManager::getSingletonPtr();
	}
protected:
};

class DaoVmCleanSetup : public TestSetup {
public:
    void setup()
    {
        DaoVm::getSingleton().pushContext();
    }
    void teardown()
	{
        DaoVm::getSingleton().popContext();
    }
protected:
};

SUITE_WITHSETUP(tDaoVm, SuiteDaoVm);

TEST_INSUITE_WITHSETUP(tDaoVm, DaoVmClean, Function)
{
    try {

        // Prepare the Dao source codes:
        DaoVm::getSingleton().execute(Path("test.dao"));
        DaoVm::getSingleton().execute(Path("test2.dao"));

        DValue dv = DaoVm::getSingleton().getReturn();

        DaoFunctionCall<int> daoR( "myfunc" ) ;
        std::tuple<int,float, const char *> args(10,2.5f,"def");
        int e = daoR(args);
        CHECK_EQUAL(123, e);

        e = DaoFunctionCall<int> ( "myfunc" )(std::make_tuple(1,2.f,String("abc")));
        CHECK_EQUAL(123, e);

        DaoFunctionCall<> daoR2("myfuncInTest2");
        daoR2(100);

        DaoFunctionCall<> daoR3("importTest");
        daoR3("from C++");

        DaoFunctionCall<> daoR3b("importTest");
        String scpp = "from C++ b";
        daoR3b(scpp);

        DaoFunctionCall<int> daoR4("regvar");
        e = daoR4("a=5");
        CHECK_EQUAL(0, e);

        DaoFunctionCall<int> daoR5("regreflect");
        e = daoR5("reflect.type(a)");
        CHECK_EQUAL(1, e);

        DaoFunctionCall<float> daoR7("getSomeFloat");
        auto dd = daoR7();
        CHECK_DOUBLES_EQUAL(123.45f, dd);
        assert( dd == 123.45f );

        DaoFunctionCall<String> daoR6("getSomeString");
        String str = daoR6();
        CHECK_EQUAL("SomeStringFromTestDao", str);

        DaoFunctionCall<double> daoR8("getSomeDouble");
        auto ddd = daoR8();
        CHECK_DOUBLES_EQUAL(123.45, ddd);

        DaoVm::getSingleton().execute(String("a=5"));
        DaoVm::getSingleton().execute(String("a"));

        bool y = DaoVm::getSingleton().containsName(  "DaoOtto" );
        CHECK( y);
        y = DaoVm::getSingleton().containsName(  "global_var");
        CHECK( y);
        y = DaoVm::getSingleton().containsName(  "global_a");
        CHECK( y);
        y = DaoVm::getSingleton().containsName(  "klaus");
        CHECK( y);

        DaoVm::getSingleton().execute(String("reflect.type(a)"));
        DaoVm::getSingleton().execute(String("reflect.type(DaoOtto)"));
        try {
            DaoFunctionCall<bool> daoTypeError("a");
            auto t = daoTypeError();
            FAIL("expected InvalidDataType exception");
        } catch ( const DaoVmInvalidDataType& ex ) {
        }

        DaoFunctionCall<> daoTypeError("a");
        auto tt = daoTypeError();

    } catch (int i) {
        FAIL(String::sfmt( "caught {1}\n" , String::nr(i)).c_str());
    } catch (char * s) {
        FAIL(String::sfmt( "caught {1}\n" , s).c_str());
    } catch (DaoVmFunctionNotFound& ex) {
        FAIL("caught FunctionNotFound\n");
    } catch (const std::exception& ex) {
        FAIL(ex.what());
    } catch (...) {
        FAIL("caught all\n");
    }

}

TEST_INSUITE_WITHSETUP(tDaoVm, DaoVmClean, Hotkeys)
{
    try {
        DaoVm::getSingleton().execute(Path("hotkeys.dao"));

        DaoVm::getSingleton().clearOutput();
        InputManager::getSingleton().simulateKeyPress(Key::F1);
        CHECK_EQUAL("exec Daokey", DaoVm::getSingleton().getOutput().trimRight()); 

        DaoVm::getSingleton().clearOutput();
        InputManager::getSingleton().simulateKeyDown(Key::ArrowDown);
        InputManager::getSingleton().simulateKeyPress(Key::F1);
        InputManager::getSingleton().simulateKeyUp(Key::ArrowDown);
        CHECK_EQUAL("doAction Daokey", DaoVm::getSingleton().getOutput().trimRight()); 

        DaoVm::getSingleton().clearOutput();
        InputManager::getSingleton().simulateKeyPress(Key::F4);
        CHECK_EQUAL("running", DaoVm::getSingleton().getOutput().trimRight()); 

        DaoVm::getSingleton().clearOutput();
        InputManager::getSingleton().simulateKeyDown(Key::LeftShift);
        InputManager::getSingleton().simulateKeyPress(Key::F4);
        InputManager::getSingleton().simulateKeyUp(Key::LeftShift);
        CHECK_EQUAL("running even faster", DaoVm::getSingleton().getOutput().trimRight()); 

        DaoVm::getSingleton().clearOutput();
        InputManager::getSingleton().simulateKeyPress(Key::F3);
        CHECK(DaoVm::getSingleton().getOutput().contains("X = ")); 
        CHECK(DaoVm::getSingleton().getOutput().contains("Y = ")); 


    } catch (DaoVmFunctionNotFound& ex) {
        printf("caught FunctionNotFound\n");
    } catch (const std::exception& ex) {
        FAIL(ex.what());
    } catch (...) {
        FAIL("caught all\n");
        printf("caught all\n");
    }
}

TEST_INSUITE_WITHSETUP(tDaoVm, DaoVmClean, Context)
{
    try {
        std::list<std::string> namesToCheck;
        // Prepare the Dao source codes:
        DaoVm::getSingleton().execute(Path("testContext.dao"));
        namesToCheck.push_back("klaus");
        namesToCheck.push_back("printgl");
        namesToCheck.push_back("SndDaoOtto");
        namesToCheck.push_back("tt");
        for( auto name = namesToCheck.begin(); name != namesToCheck.end(); ++name ) {
            CHECK(DaoVm::getSingleton().containsName((*name).c_str()));
        }

        DaoVm::getSingleton().execute(String("karl = Hotkey(Key::F1, SndDaoOtto)"));
        CHECK( DaoVm::getSingleton().containsName("karl") );
        DValue dv = DaoVm::getSingleton().getReturn();
        DaoVm::getSingleton().pushContext();
        ///////////////////////////////////////////////////////////
        DaoVm::getSingleton().execute(Path("process2.dao"));
        DaoVm::getSingleton().clearOutput();
        DaoFunctionCall<>("P2Routine")(5);
        CHECK(DaoVm::getSingleton().getOutput().contains("Dao P2: in P2Rountine() in=5")); 
        namesToCheck.clear();
        namesToCheck.push_back("klaus");
        namesToCheck.push_back("tt");
        namesToCheck.push_back("karl");
        namesToCheck.push_back("printgl");
        namesToCheck.push_back("SndDaoOtto");
        namesToCheck.push_back("P2Otto");
        namesToCheck.push_back("p2obj");
        namesToCheck.push_back("p2snd");
        namesToCheck.push_back("P2Routine");

        for( auto name = namesToCheck.begin(); name != namesToCheck.end(); ++name ) {
            if( !DaoVm::getSingleton().containsName((*name).c_str()) ) {
                FAIL( String::sfmt("ERROR: Don't find {1} in new namespace", *name).c_str() );
            }
        }
        DaoVm::getSingleton().execute(String("p2test = SndDaoOtto()\np2test.name()"));
        CHECK( DaoVm::getSingleton().containsName("p2test") ) ;

        ///////////////////////////////////////////////////////////
        DaoVm::getSingleton().popContext();
        namesToCheck.clear();
        namesToCheck.push_back("SndDaoOtto");
        namesToCheck.push_back("objSnd"); // it is public global and imported from test2.dao
        //namesToCheck.push_back("objotto2"); it is not global and can't be accessed since test2 is loaded/imported.
        namesToCheck.push_back("DaoOtto");
        namesToCheck.push_back("karl");
        namesToCheck.push_back("tt");
        namesToCheck.push_back("klaus");

        for( auto name = namesToCheck.begin(); name != namesToCheck.end(); ++name ) {
            if( !DaoVm::getSingleton().containsName((*name).c_str()) ) {
                FAIL( String::sfmt("ERROR: Don't find {1} in restored namespace", *name).c_str() );
            }
        }
        namesToCheck.clear();
        namesToCheck.push_back("P2Otto");
        namesToCheck.push_back("p2obj");
        namesToCheck.push_back("p2snd");
        namesToCheck.push_back("P2Routine");
        for( auto name = namesToCheck.begin(); name != namesToCheck.end(); ++name ) {
            if( DaoVm::getSingleton().containsName((*name).c_str()) ) {
                FAIL(String::sfmt("ERROR! found {1} of destroyed namespace", (*name).c_str()).c_str());
            }
        }
        DaoVm::getSingleton().clearOutput();
        DaoFunctionCall<>("printgl")(77);
        CHECK(DaoVm::getSingleton().getOutput().contains("global_a= 77")); 


    } catch (DaoVmFunctionNotFound& ex) {
        printf("caught FunctionNotFound\n");
    } catch (const std::exception& ex) {
        FAIL(ex.what());
    } catch (...) {
        FAIL("caught all\n");
        printf("caught all\n");
    }
}
