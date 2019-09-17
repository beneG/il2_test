#pragma once
#include <algorithm>
#include <memory>
#include <string>

class Cockroach;
class Food
{
public:
	constexpr static const double DEFAULT_SIZE = 40.0;
	constexpr static const double PIECE_SIZE = 0.1;

	Food(int x, int y) : m_x(x), m_y(y), m_size(DEFAULT_SIZE) { }

	void eat()
	{
		m_size -= PIECE_SIZE;
	}

	void draw(HDC hdc, std::list<std::shared_ptr<Cockroach>> &cockroaches)
	{
		int percent = (int)(m_size * 100 / DEFAULT_SIZE);
		auto eaters_count = std::count_if(cockroaches.begin(), cockroaches.end(), [this](auto cockroach) {
			if (cockroach->get_status() == Cockroach::Status::CONSUMING
				&& cockroach->distance(m_x, m_y) < cockroach->get_radius() + m_size + 10) {
				return true;
			}
			return false;
		});

		std::wstring status_str = L"food: " + std::to_wstring(percent) + L"%";
		std::wstring eaters_count_str = L"eaters: " + std::to_wstring(eaters_count);

		TextOut(hdc, (int)(m_x + m_size / 2), (int)(m_y - m_size / 2), status_str.c_str(), (int)status_str.length());
		TextOut(hdc, (int)(m_x + m_size / 2), (int)(m_y - m_size / 2 + 10), eaters_count_str.c_str(), (int)eaters_count_str.length());

		Rectangle(hdc, (int)(m_x - m_size / 2), (int)(m_y - m_size / 2), (int)(m_x + m_size / 2), (int)(m_y + m_size / 2));
	}

	inline double get_size() { return m_size; }
	inline int get_x() { return m_x; }
	inline int get_y() { return m_y; }

private:
	double m_size;
	int m_x;
	int m_y;
};


class FoodFactory
{
public:
	static std::shared_ptr<Food> make_food(int x, int y) {
		return std::make_shared<Food>(x, y);
	}
};
