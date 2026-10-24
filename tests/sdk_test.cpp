#include <slam/slam.hpp>

#include <catch2/catch_test_macros.hpp>

namespace {

auto Add(Slam::int32 A, Slam::int32 B) -> Slam::int32 { return A + B; }
auto Enabled(bool Value) -> bool { return Value; }

SLAM_BIND(Add);
SLAM_BIND(Enabled);

struct FTestApplication final : Slam::FApplication {
  void BeginPlay() override { ++Begins; }
  void Tick(float DeltaTime) override { LastDelta = DeltaTime; }
  void EndPlay() override { ++Ends; }
  static inline int Begins = 0;
  static inline int Ends = 0;
  static inline float LastDelta = 0.0F;
};

SLAM_IMPLEMENT_APPLICATION(FTestApplication);

} // namespace

TEST_CASE("typed SDK bindings convert and invoke values") {
  REQUIRE(std::string(Slam::slam_invoke("Add", "1:21:3")) == "0:5");
  REQUIRE(std::string(Slam::slam_invoke("Enabled", "5:false")) == "0:false");
  REQUIRE(std::string(Slam::slam_invoke("Add", "3:bad1:2")).find("1:") == 0);
  REQUIRE(std::string(Slam::slam_invoke("Missing", "")).find("1:") == 0);
}

TEST_CASE("application registration owns the browser lifecycle") {
  FTestApplication::Begins = 0;
  FTestApplication::Ends = 0;
  Slam::slam_start_application();
  Slam::slam_tick(0.016F);
  REQUIRE(FTestApplication::Begins == 1);
  REQUIRE(FTestApplication::LastDelta == 0.016F);
  Slam::slam_stop_application();
  REQUIRE(FTestApplication::Ends == 1);
}

TEST_CASE("native SDK platform helpers report unavailable browser state") {
  REQUIRE_FALSE(Slam::FInput::IsKeyDown(Slam::EKey::SpaceBar));
  REQUIRE(Slam::FDisplay::GetOrientation() == Slam::EOrientation::Unknown);
  REQUIRE_FALSE(Slam::FCanvas::RequestFullscreen());
}
