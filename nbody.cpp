/**
 *  Symplectic integrator, equivalent to the implementation used for the
 *  Computer Language Benchmarks Game n-body problem.
 *  Author: Quinten van Woerkom
 */

#include <array>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <type_traits>
#include <vector>

#include "utility.h"

constexpr auto pi = 3.141592653589793;
constexpr auto solar_mass = 4*pi*pi;
constexpr auto days_per_year = 365.24;


/**
 *  Three-dimensional vector, supporting only the operations that are
 *  strictly required for this symplectic integrator.
 */
class vector3d {
public:
  template<typename... Args>
  constexpr vector3d(Args... args) : data{args...} {}

  constexpr auto squared_norm() const noexcept -> double {
    return data[0]*data[0] + data[1]*data[1] + data[2]*data[2];
  }

  constexpr auto norm() const noexcept -> double {
    return std::sqrt(squared_norm());
  }

  constexpr auto operator[](const std::size_t index) noexcept -> double& {
    return data[index];
  }
  
  constexpr auto operator[](const std::size_t index) const noexcept -> const double& {
    return data[index];
  }

  constexpr auto operator+=(const vector3d& rhs) noexcept -> vector3d& {
    data[0] += rhs[0];
    data[1] += rhs[1];
    data[2] += rhs[2];
    return *this;
  }

  constexpr auto operator-=(const vector3d& rhs) noexcept -> vector3d& {
    data[0] -= rhs[0];
    data[1] -= rhs[1];
    data[2] -= rhs[2];
    return *this;
  }

private:
  std::array<double, 3> data = {0.0, 0.0, 0.0};
};

constexpr auto operator+(vector3d lhs, const vector3d& rhs) noexcept -> vector3d {
  lhs += rhs;
  return lhs;
}

constexpr auto operator-(vector3d lhs, const vector3d& rhs) noexcept -> vector3d {
  lhs -= rhs;
  return lhs;
}

constexpr auto operator*(const vector3d& vector, double factor) noexcept -> vector3d {
  return {vector[0]*factor, vector[1]*factor, vector[2]*factor};
};

constexpr auto operator/(const vector3d& vector, double divisor) noexcept -> vector3d {
  return {vector[0]/divisor, vector[1]/divisor, vector[2]/divisor};
}


/**
 *  Celestial body; has a mass, position, and velocity.
 */
class body {
public:
  constexpr body(const vector3d& x, const vector3d& v, const double m)
    : position{x}, velocity{v}, mass{m} {}

  constexpr auto advance(const double dt) noexcept -> body& {
    position += velocity*dt;
    return *this;
  }

  constexpr auto adjust_momentum(const vector3d momentum) noexcept -> body& {
    velocity -= momentum/mass;
    return *this;
  }

  constexpr void correct(body& other, const double dt) noexcept {
    const auto dx = relative_position(other);
    const auto distance = dx.squared_norm();
    const auto correction = dt / (distance*std::sqrt(distance));
    velocity -= dx*other.mass*correction;
    other.velocity += dx*mass*correction;
  }

  constexpr auto momentum() const noexcept -> vector3d {
    return velocity*mass;
  }

  constexpr auto kinetic_energy() const noexcept -> double {
    return 0.5*mass*velocity.squared_norm();
  }

  constexpr auto potential_energy(const body& other) const noexcept -> double {
    const auto distance = relative_position(other).norm();
    return -(mass*other.mass)/distance;
  }

  constexpr auto relative_position(const body& other) const noexcept -> vector3d {
    return position - other.position;
  }

private:
  vector3d position;
  vector3d velocity;
  double mass;
};


/**
 *  A system of celestial bodies, with the first body considered to be the
 *  gravitationally dominant member.
 *  Size can be determined from the constructor argument list at compile time,
 *  or be kept dynamic using N=-1.
 */
template<int N = -1>
class solar_system {
public:
  template<typename... Args>
  solar_system(Args... args)
    : bodies{args...} {
    // Make sure the total momentum of the system is zero.
    bodies[0].adjust_momentum(momentum());
  }

  void advance(const double dt) noexcept {
    for (auto [left, right] : pairwise_combinations(bodies))
      left.correct(right, dt);
    for (auto& body : bodies)
      body.advance(dt);
  }

  auto kinetic_energy() const noexcept -> double {
    double total = 0.0;
    for (const auto& body : bodies)
      total += body.kinetic_energy();
    return total;
  }

  auto potential_energy() const noexcept -> double {
    double total = 0.0;
    for (const auto [left, right] : pairwise_combinations(bodies))
      total += left.potential_energy(right);
    return total;
  }

  auto energy() const noexcept -> double {
    return kinetic_energy() + potential_energy();
  }

  auto momentum() const noexcept -> vector3d {
    auto momentum = vector3d{};
    for (const auto& body : bodies)
      momentum += body.momentum();
    return momentum;
  }

private:
  using container = std::conditional_t<N == -1, std::vector<body>, std::array<body, N>>;
  container bodies;
};

template<typename... Args>
solar_system(Args... args) -> solar_system<sizeof...(args)>;


/**
 *  Initial position, velocity, and mass values of all celestial bodies considered.
 *  In this example, the sun and the four gas giants are included.
 */
constexpr auto sol = body{
  {0.0, 0.0, 0.0},
  {0.0, 0.0, 0.0},
  solar_mass
};

constexpr auto jupiter = body{
  vector3d{ 4.84143144246472090e+00, -1.16032004402742839e+00, -1.03622044471123109e-01},
  vector3d{ 1.66007664274403694e-03,  7.69901118419740425e-03, -6.90460016972063023e-05}*days_per_year,
  9.54791938424326609e-04*solar_mass
};

constexpr auto saturn = body{
  vector3d{ 8.34336671824457987e+00,  4.12479856412430479e+00, -4.03523417114321381e-01},
  vector3d{-2.76742510726862411e-03,  4.99852801234917238e-03,  2.30417297573763929e-05}*days_per_year,
  2.85885980666130812e-04*solar_mass
};

constexpr auto uranus = body{
  vector3d{ 1.28943695621391310e+01, -1.51111514016986312e+01, -2.23307578892655734e-01},
  vector3d{ 2.96460137564761618e-03,  2.37847173959480950e-03, -2.96589568540237556e-05}*days_per_year,
  4.36624404335156298e-05*solar_mass
};

constexpr auto neptune = body{
  vector3d{ 1.53796971148509165e+01, -2.59193146099879641e+01,  1.79258772950371181e-01},
  vector3d{ 2.68067772490389322e-03,  1.62824170038242295e-03, -9.51592254519715870e-05}*days_per_year,
  5.15138902046611451e-05*solar_mass
};


int main(int argc, char** argv) {
  auto sys = solar_system{sol, jupiter, saturn, uranus, neptune};
  int n = 1000;
  if (argc > 1)
    n = std::atoi(argv[1]);
  
  std::cout << std::setprecision(9) << sys.energy() << '\n';
  for (auto i = 0; i < n; ++i)
    sys.advance(0.01);
  std::cout << std::setprecision(9) << sys.energy() << '\n';

  return 0;
}