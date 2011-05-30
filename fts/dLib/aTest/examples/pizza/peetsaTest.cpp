#include "../../TestHarness.h"
#include "peetsa.h"

TEST(PeetsaWithoutDoughThrows)
{
    try {
        // This is expected to throw.
        Peetsa p(NULL);

        // Thus we should never reach the following code ; if we reach it, the
        // above didn't throw and our test should record a failure.
        FAIL("Peetsa p(NULL) doesn't throw an exception!");
    } catch(const std::exception& e) {
        // It is good to come here.
    }
}

TEST(PeetsaGetsDough)
{
    ThinItalianDough *d = new ThinItalianDough(PeetsaSize::Big);
    Peetsa p(d);

    CHECK_EQUAL(*d, p.getDough());
}

TEST(PeetsaGetsPizzaHutDough)
{
    PizzaHutDough *d = new PizzaHutDough(PeetsaSize::Medium, 437);
    Peetsa p(d);

    // This test will fail. That's what we expect.
    // Note that the operator== from derived classes needs to have the same
    // signature as the operator== of its base-class.
//     CHECK_EQUAL(PizzaHutDough(PeetsaSize::Medium, 1), p.getDough());
    CHECK_EQUAL(*d, p.getDough());
}

TEST(PeetsaWithoutSauceThrows)
{
    try {
        // This is expected to throw.
        Peetsa p(new ThinItalianDough(PeetsaSize::Medium));
        p.addSauce(NULL);

        // Thus we should never reach the following code ; if we reach it, the
        // above didn't throw and our test should record a failure.
        FAIL("Adding NULL sauce to the Peetsa p.addSauce(NULL) doesn't throw an exception!");
    } catch(...) {
        // It is good to come here.
    }
}

TEST(PeetsaGetsTomatoeSauce)
{
    Peetsa p(new ThinItalianDough(PeetsaSize::Medium));
    p.addSauce(new TomatoeSauce(0.5));

    CHECK_EQUAL(TomatoeSauce(0.6), p.getSauce());
}

TEST(PeetsaGetsHotChiliSauce)
{
    Peetsa p(new PizzaHutDough(PeetsaSize::Big, 437));
    p.addSauce(new HotChiliSauce(ChiliHotness::Hot, 0.3));

    CHECK_EQUAL(HotChiliSauce(ChiliHotness::Hot, 0.3), p.getSauce());
}
