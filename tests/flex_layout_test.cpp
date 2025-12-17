// tests/flex_layout_test.cpp - Quick Verification Test
#include "frqs-widget.hpp"
#include "widget/container.hpp"
#include "widget/label.hpp"
#include <print>
#include <cassert>

using namespace frqs;
using namespace frqs::widget;

template <>
struct std::formatter<frqs::widget::LayoutProps::Align> : std::formatter<string_view> {
    auto format(frqs::widget::LayoutProps::Align a, std::format_context& ctx) const {
        std::string_view name = "Unknown";
        switch (a) {
            case frqs::widget::LayoutProps::Align::Start:   name = "Start"; break;
            case frqs::widget::LayoutProps::Align::Center:  name = "Center"; break;
            case frqs::widget::LayoutProps::Align::End:     name = "End"; break;
            case frqs::widget::LayoutProps::Align::Stretch: name = "Stretch"; break;
        }
        return std::formatter<string_view>::format(name, ctx);
    }
};

// ============================================================================
// TEST HELPERS
// ============================================================================

#define ASSERT_EQ(a, b) \
    if ((a) != (b)) { \
        std::println(stderr, "Assertion failed: {} != {}", #a, #b); \
        std::println(stderr, "  Expected: {}", b); \
        std::println(stderr, "  Got: {}", a); \
        std::terminate(); \
    }

#define ASSERT_NEAR(a, b, epsilon) \
    if (std::abs((a) - (b)) > (epsilon)) { \
        std::println(stderr, "Assertion failed: {} not near {}", #a, #b); \
        std::println(stderr, "  Expected: {}", b); \
        std::println(stderr, "  Got: {}", a); \
        std::terminate(); \
    }

// ============================================================================
// TEST 1: Layout Properties Storage
// ============================================================================

void test_layout_props_storage() {
    std::println("TEST: Layout properties storage");
    
    auto widget = std::make_shared<Widget>();
    
    // Default values
    ASSERT_EQ(widget->getLayoutWeight(), 0.0f);
    ASSERT_EQ(widget->getAlignSelf(), LayoutProps::Align::Stretch);
    
    // Set values
    widget->setLayoutWeight(2.5f);
    widget->setMinWidth(100);
    widget->setMaxWidth(500);
    widget->setAlignSelf(LayoutProps::Align::Center);
    
    // Verify
    ASSERT_EQ(widget->getLayoutWeight(), 2.5f);
    ASSERT_EQ(widget->getLayoutProps().minWidth, 100);
    ASSERT_EQ(widget->getLayoutProps().maxWidth, 500);
    ASSERT_EQ(widget->getAlignSelf(), LayoutProps::Align::Center);
    
    std::println("  ✓ Layout properties stored correctly\n");
}

// ============================================================================
// TEST 2: Fixed Size Allocation
// ============================================================================

void test_fixed_size_allocation() {
    std::println("TEST: Fixed size allocation (weight = 0)");
    
    auto container = createFlexRow(0, 0);
    container->setRect(Rect(0, 0, 600u, 100u));
    
    // Create two fixed-size widgets
    auto w1 = std::make_shared<Widget>();
    w1->setLayoutWeight(0.0f);
    w1->setRect(Rect(0, 0, 200u, 100u));
    
    auto w2 = std::make_shared<Widget>();
    w2->setLayoutWeight(0.0f);
    w2->setRect(Rect(0, 0, 150u, 100u));
    
    container->addChild(w1);
    container->addChild(w2);
    
    // Apply layout
    container->applyLayout();
    
    // Widgets should keep their sizes
    ASSERT_EQ(w1->getRect().w, 200u);
    ASSERT_EQ(w2->getRect().w, 150u);
    
    // Check positioning (should be sequential)
    ASSERT_EQ(w1->getRect().x, 0);
    ASSERT_EQ(w2->getRect().x, 200);  // After w1
    
    std::println("  ✓ Fixed sizes preserved");
    std::println("  ✓ Sequential positioning correct\n");
}

// ============================================================================
// TEST 3: Flex Distribution (Equal Weights)
// ============================================================================

void test_flex_distribution_equal() {
    std::println("TEST: Flex distribution (equal weights)");
    
    auto container = createFlexRow(0, 0);
    container->setRect(Rect(0, 0, 600u, 100u));
    
    // Create three widgets with equal weight
    auto w1 = std::make_shared<Widget>();
    w1->setLayoutWeight(1.0f);
    
    auto w2 = std::make_shared<Widget>();
    w2->setLayoutWeight(1.0f);
    
    auto w3 = std::make_shared<Widget>();
    w3->setLayoutWeight(1.0f);
    
    container->addChild(w1);
    container->addChild(w2);
    container->addChild(w3);
    
    // Apply layout
    container->applyLayout();
    
    // Each widget should get 1/3 of space (200px)
    ASSERT_EQ(w1->getRect().w, 200u);
    ASSERT_EQ(w2->getRect().w, 200u);
    ASSERT_EQ(w3->getRect().w, 200u);
    
    // Check positioning
    ASSERT_EQ(w1->getRect().x, 0);
    ASSERT_EQ(w2->getRect().x, 200);
    ASSERT_EQ(w3->getRect().x, 400);
    
    std::println("  ✓ Space distributed equally");
    std::println("  ✓ Each widget: 200px (1/3 of 600px)\n");
}

// ============================================================================
// TEST 4: Flex Distribution (Weighted)
// ============================================================================

void test_flex_distribution_weighted() {
    std::println("TEST: Flex distribution (weighted 1:2:1)");
    
    auto container = createFlexRow(0, 0);
    container->setRect(Rect(0, 0, 800u, 100u));
    
    // Create widgets with weights 1, 2, 1
    auto w1 = std::make_shared<Widget>();
    w1->setLayoutWeight(1.0f);
    
    auto w2 = std::make_shared<Widget>();
    w2->setLayoutWeight(2.0f);  // Double!
    
    auto w3 = std::make_shared<Widget>();
    w3->setLayoutWeight(1.0f);
    
    container->addChild(w1);
    container->addChild(w2);
    container->addChild(w3);
    
    // Apply layout
    container->applyLayout();
    
    // w1: 1/4 = 200px
    // w2: 2/4 = 400px (double!)
    // w3: 1/4 = 200px
    ASSERT_EQ(w1->getRect().w, 200u);
    ASSERT_EQ(w2->getRect().w, 400u);
    ASSERT_EQ(w3->getRect().w, 200u);
    
    std::println("  ✓ Weighted distribution correct");
    std::println("  ✓ Sizes: 200px, 400px, 200px (1:2:1)\n");
}

// ============================================================================
// TEST 5: Mixed Fixed + Flex
// ============================================================================

void test_mixed_fixed_flex() {
    std::println("TEST: Mixed fixed + flex layout");
    
    auto container = createFlexRow(0, 0);
    container->setRect(Rect(0, 0, 1000u, 100u));
    
    // Fixed sidebar (250px)
    auto sidebar = std::make_shared<Widget>();
    sidebar->setLayoutWeight(0.0f);
    sidebar->setRect(Rect(0, 0, 250u, 100u));
    
    // Flex content (should get remaining 750px)
    auto content = std::make_shared<Widget>();
    content->setLayoutWeight(1.0f);
    
    container->addChild(sidebar);
    container->addChild(content);
    
    // Apply layout
    container->applyLayout();
    
    // Sidebar: fixed 250px
    // Content: remaining 750px (1000 - 250)
    ASSERT_EQ(sidebar->getRect().w, 250u);
    ASSERT_EQ(content->getRect().w, 750u);
    
    std::println("  ✓ Fixed widget: 250px");
    std::println("  ✓ Flex widget: 750px (remaining space)\n");
}

// ============================================================================
// TEST 6: Constraints (Min/Max Width)
// ============================================================================

void test_constraints() {
    std::println("TEST: Size constraints (min/max)");
    
    auto container = createFlexRow(0, 0);
    container->setRect(Rect(0, 0, 1000u, 100u));
    
    // Widget with max constraint
    auto widget = std::make_shared<Widget>();
    widget->setLayoutWeight(1.0f);
    widget->setMaxWidth(300);  // Clamp to 300px max
    
    container->addChild(widget);
    
    // Apply layout
    container->applyLayout();
    
    // Should be clamped to maxWidth
    ASSERT_EQ(widget->getRect().w, 300u);
    
    std::println("  ✓ Max width constraint applied (300px)\n");
}

// ============================================================================
// TEST 7: Column Direction (Vertical)
// ============================================================================

void test_column_direction() {
    std::println("TEST: Column direction (vertical layout)");
    
    auto container = createFlexColumn(0, 0);
    container->setRect(Rect(0, 0, 200u, 600u));
    
    // Create two widgets with equal weight
    auto w1 = std::make_shared<Widget>();
    w1->setLayoutWeight(1.0f);
    
    auto w2 = std::make_shared<Widget>();
    w2->setLayoutWeight(1.0f);
    
    container->addChild(w1);
    container->addChild(w2);
    
    // Apply layout
    container->applyLayout();
    
    // Each should get half the height (300px)
    ASSERT_EQ(w1->getRect().h, 300u);
    ASSERT_EQ(w2->getRect().h, 300u);
    
    // Check vertical positioning
    ASSERT_EQ(w1->getRect().y, 0);
    ASSERT_EQ(w2->getRect().y, 300);
    
    std::println("  ✓ Vertical distribution correct");
    std::println("  ✓ Each widget: 300px height\n");
}

// ============================================================================
// TEST 8: Gap Spacing
// ============================================================================

void test_gap_spacing() {
    std::println("TEST: Gap spacing between items");
    
    auto container = createFlexRow(20, 0);  // 20px gap
    container->setRect(Rect(0, 0, 640u, 100u));
    
    // Three equal widgets
    auto w1 = std::make_shared<Widget>();
    w1->setLayoutWeight(1.0f);
    
    auto w2 = std::make_shared<Widget>();
    w2->setLayoutWeight(1.0f);
    
    auto w3 = std::make_shared<Widget>();
    w3->setLayoutWeight(1.0f);
    
    container->addChild(w1);
    container->addChild(w2);
    container->addChild(w3);
    
    // Apply layout
    container->applyLayout();
    
    // Available: 640 - (2 * 20 gap) = 600px
    // Each: 600 / 3 = 200px
    ASSERT_EQ(w1->getRect().w, 200u);
    ASSERT_EQ(w2->getRect().w, 200u);
    ASSERT_EQ(w3->getRect().w, 200u);
    
    // Check spacing
    ASSERT_EQ(w1->getRect().x, 0);
    ASSERT_EQ(w2->getRect().x, 220);  // 200 + 20 gap
    ASSERT_EQ(w3->getRect().x, 440);  // 220 + 200 + 20 gap
    
    std::println("  ✓ Gap spacing applied correctly (20px)\n");
}

// ============================================================================
// MAIN TEST RUNNER
// ============================================================================

int main() {
    try {
        std::println("=== FRQS-Widget FlexLayout Tests ===\n");
        
        test_layout_props_storage();
        test_fixed_size_allocation();
        test_flex_distribution_equal();
        test_flex_distribution_weighted();
        test_mixed_fixed_flex();
        test_constraints();
        test_column_direction();
        test_gap_spacing();
        
        std::println("=================================");
        std::println("✅ ALL TESTS PASSED!");
        std::println("=================================\n");
        
        std::println("FlexLayout implementation verified:");
        std::println("  • Layout properties storage: ✓");
        std::println("  • Fixed size allocation: ✓");
        std::println("  • Flex distribution: ✓");
        std::println("  • Weighted distribution: ✓");
        std::println("  • Mixed layouts: ✓");
        std::println("  • Size constraints: ✓");
        std::println("  • Column direction: ✓");
        std::println("  • Gap spacing: ✓");
        
        return 0;
        
    } catch (const std::exception& e) {
        std::println(stderr, "\n❌ TEST FAILED: {}", e.what());
        return 1;
    }
}