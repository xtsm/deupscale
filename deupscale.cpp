#include "deupscale.h"
#include <cassert>
#include <algorithm>
#include <set>
#include <span>
#include <vector>

#include <iostream>


constexpr bool debug = false;


int DiffL2(const std::span<unsigned char>& a, const std::span<unsigned char>& b) {
  assert(a.size() == b.size());
  int diff = 0;
  for (size_t i = 0; i < a.size(); ++i) {
    int d = a[i];
    d -= b[i];
    diff += d * d;
  }
  return diff;
}

struct Point {
  double x, y;
};

std::ostream& operator<<(std::ostream& os, const Point& p) {
  return os << p.x << ',' << p.y;
}

struct Segment {
  // b = pk+q {l <= k <= r}
  double p, q, l, r;

  Point At(const double k) const {
    return Point{k, p * k + q};
  }

  Point LeftEnd() const {
    return At(l);
  }

  Point RightEnd() const {
    return At(r);
  }
};

// true if b covers a in bottom convex hull
bool OutOfRangeRight(const Segment& a, const Segment& b) {
  assert(a.p > b.p);
  // a.p * k + a.q = b.p * k + b.q
  // k = (b.q - a.q) / (a.p - b.p)
  // k >= a.r
  return b.q - a.q >= a.r * (a.p - b.p);
}

// true if b covers a in top convex hull
bool OutOfRangeLeft(const Segment& a, const Segment& b) {
  assert(a.p > b.p);
  // a.p * k + a.q = b.p * k + b.q
  // k = (b.q - a.q) / (a.p - b.p)
  // k <= a.l
  return b.q - a.q <= a.l * (a.p - b.p);
}

double IntersectK(const Segment& a, const Segment& b) {
  assert(a.p != b.p);
  // a.p * k + a.q = b.p * k + b.q
  return (b.q - a.q) / (a.p - b.p);
}

double TriangleArea(const Point& p1, const Point& p2, const Point& p3) {
  return 0.5 * ((p2.x - p1.x) * (p3.y - p1.y) - (p2.y - p1.y) * (p3.x - p1.x));
}

struct AxisResult {
  bool success;
  double k, b;
  double min_k, max_k;
  double min_b, max_b;
  double hypothesis_area;
};

AxisResult SolveAxis(const std::vector<int>& diffs, int threshold) {
  std::vector<Segment> bottom, top;
  const auto add_bottom = [&bottom](double p, double q) {
    Segment bs{p, q, 0, 1};
    while (!bottom.empty() && OutOfRangeRight(bottom.back(), bs)) {
      bottom.pop_back();
      if (!bottom.empty()) {
        bottom.back().l = 0;
      }
    }
    if (bottom.empty()) {
      bottom.push_back(bs);
    } else if (!OutOfRangeLeft(bottom.back(), bs)) {
      double k = IntersectK(bottom.back(), bs);
      bottom.back().l = k;
      bs.r = k;
      bottom.push_back(bs);
    }
  };
  const auto add_top = [&top](double p, double q) {
    Segment ts{p, q, 0, 1};
    while (!top.empty() && OutOfRangeLeft(top.back(), ts)) {
      top.pop_back();
      if (!top.empty()) {
        top.back().r = 1;
      }
    }
    if (top.empty()) {
      top.push_back(ts);
    } else if (!OutOfRangeRight(top.back(), ts)) {
      double k = IntersectK(top.back(), ts);
      top.back().r = k;
      ts.l = k;
      top.push_back(ts);
    }
  };
  
  add_bottom(0, 0);
  add_top(0, 1);

  size_t j = 0;
  for (size_t i = 1; i < diffs.size() + 1; ++i) {
    if (diffs[i - 1] > threshold) {
      ++j;
    }
    // i of transformed maps to j of original
    // floor(i*k+b) = j
    // j <= i*k+b < j+1
    
    add_bottom(-static_cast<double>(i), static_cast<double>(j));
    add_top(-static_cast<double>(i), static_cast<double>(j+1));
  }

  if (debug) {
    std::cerr << "Bottom convex hull:" << std::endl;
    std::cerr << bottom.front().RightEnd() << std::endl;
    for (const auto& seg : bottom) {
      std::cerr << seg.LeftEnd() << std::endl;
    }

    std::cerr << "Top convex hull:" << std::endl;
    std::cerr << top.front().LeftEnd() << std::endl;
    for (const auto& seg : top) {
      std::cerr << seg.RightEnd() << std::endl;
    }
  }

  auto bi = bottom.rbegin(), be = bottom.rend();
  auto ti = top.begin(), te = top.end();
  double sl = bottom.back().LeftEnd().y <= top.front().LeftEnd().y ? 0 : 1;
  double sr = bottom.front().RightEnd().y <= top.back().RightEnd().y ? 1 : 0;
  while (bi != be && ti != te) {
    if (bi->p == ti->p) {
      if (bi->q == ti->q) {
        sl = std::min(sl, std::max(bi->l, ti->l));
        sr = std::max(sr, std::min(bi->r, ti->r));
      }
    } else {
      const auto k = IntersectK(*bi, *ti);
      if (bi->l <= k && ti->l <= k && bi->r >= k && ti->r >= k) {
        sl = std::min(sl, k);
        sr = std::max(sr, k);
      }
    }
    
    if (bi->r < ti->r) {
      ++bi;
    } else {
      ++ti;
    }
  }

  std::vector<Point> hyp;
  for (const auto& seg : bottom) {
    if (std::max(seg.l, sl) <= std::min(seg.r, sr)) {
      hyp.push_back(seg.At(std::min(seg.r, sr)));
      hyp.push_back(seg.At(std::max(seg.l, sl)));
    }
  }
  for (const auto& seg : top) {
    if (std::max(seg.l, sl) <= std::min(seg.r, sr)) {
      hyp.push_back(seg.At(std::max(seg.l, sl)));
      hyp.push_back(seg.At(std::min(seg.r, sr)));
    }
  }
  
  if (debug) {
    std::cerr << "Hypothesis polygon:" << std::endl;
    for (const auto& pt : hyp) {
      std::cerr << pt << std::endl;
    }
  }

  if (hyp.empty()) {
    return {false};
  }
  AxisResult res;
  res.success = true;

  res.min_k = res.max_k = hyp.front().x;
  res.min_b = res.max_b = hyp.front().y;
  for (const auto& pt : hyp) {
    res.min_k = std::min(res.min_k, pt.x); 
    res.max_k = std::max(res.max_k, pt.x); 
    res.min_b = std::min(res.min_b, pt.y); 
    res.max_b = std::max(res.max_b, pt.y); 
  }
  res.hypothesis_area = 0;
  
  if (hyp.size() == 1) {
    res.k = res.min_k;
    res.b = res.min_b;
    return res;
  }

  Point mid{0, 0};
  for (size_t i = 1; i + 1 < hyp.size(); ++i) {
    const auto area = TriangleArea(hyp[0], hyp[i], hyp[i + 1]);
    mid.x += (hyp[0].x + hyp[i].x + hyp[i+1].x) / 3.0 * area;
    mid.y += (hyp[0].y + hyp[i].y + hyp[i+1].y) / 3.0 * area;
    res.hypothesis_area += area;
  }
  res.k = mid.x / res.hypothesis_area;
  res.b = mid.y / res.hypothesis_area;
  return res;
}

bool Deupscale(unsigned char* data_, size_t* width_ptr, size_t* height_ptr, size_t channels) {
  const auto width = *width_ptr;
  const auto height = *height_ptr;
  std::span<unsigned char> data(data_, width * height * channels);

  std::vector<int> max_diff_x(width - 1);
  for (size_t i = 0; i < height; ++i) {
    for (size_t j = 0; j < width - 1; ++j) {
      max_diff_x[j] = std::max(max_diff_x[j], DiffL2(
          data.subspan((i * width + j) * channels, channels), 
          data.subspan((i * width + j + 1) * channels, channels)));
    }
  }
  std::vector<int> max_diff_y(height - 1);
  for (size_t i = 0; i < height - 1; ++i) {
    for (size_t j = 0; j < width; ++j) {
      max_diff_y[i] = std::max(max_diff_y[i], DiffL2(
          data.subspan((i * width + j) * channels, channels), 
          data.subspan((i * width + j + width) * channels, channels)));
    }
  }

  const auto res_x = SolveAxis(max_diff_x, /* threshold = */ 0);
  if (!res_x.success) {
    std::cerr << "x axis failed" << std::endl;
    return false;
  }
  const auto res_y = SolveAxis(max_diff_y, /* threshold = */ 0);
  if (!res_x.success) {
    std::cerr << "y axis failed" << std::endl;
    return false;
  }
  if (debug) {
    std::cerr << "Hypothesis areas: "
        << res_x.hypothesis_area << ","
        << res_y.hypothesis_area << std::endl;
  }

  const size_t new_width = (width - 1) * res_x.k + res_x.b + 1;
  const size_t new_height = (height - 1) * res_y.k + res_y.b + 1;
  if (debug) {
    std::cerr << "New size: " << new_width << "," << new_height << std::endl;
  }
  for (size_t i = 0; i < height; ++i) {
    size_t ii = i * res_y.k + res_y.b;
    for (size_t j = 0; j < width; ++j) {
      size_t jj = j * res_x.k + res_x.b;
      auto span_to = data.subspan((ii * new_width + jj) * channels, channels);
      auto span_from = data.subspan((i * width + j) * channels, channels);
      std::copy(span_from.begin(), span_from.end(), span_to.begin());
    }
  }

  *width_ptr = new_width;
  *height_ptr = new_height;
  return true;
}
