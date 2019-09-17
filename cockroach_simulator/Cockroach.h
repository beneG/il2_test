#pragma once

#include <Windows.h>
#include <cmath>
#include <complex>
#include <memory>
#include <list>
#include <string>

#include "static_global_vars.h"
#include "Food.h"


class Cockroach
{
public:
	constexpr static const int DEFAULT_RADIUS = 10;
	constexpr static const int MAX_RADIUS = 20;
	constexpr static const int SIGHT_RADIUS = DEFAULT_RADIUS * 10;
	constexpr static const double DEFAULT_VELOCITY = 4.0;

	enum class Status {
		NORMAL,
		FRIGHTENED,
		CONSUMING,
		FULL,
		EXPIRED
	};

	Cockroach(int x, int y) : m_x(x), m_y(y)
	{
		m_radius = DEFAULT_RADIUS;
		m_velocity = DEFAULT_VELOCITY;
		m_status = Status::NORMAL;
		m_spawn_time = std::chrono::system_clock::now();
	}

	~Cockroach()
	{}

	void draw(HDC hdc)
	{
		std::wstring status;
		switch (m_status)
		{
		case Cockroach::Status::NORMAL:
			status = L"searching";
			break;
		case Cockroach::Status::FRIGHTENED:
			status = L"scared";
			break;
		case Cockroach::Status::CONSUMING:
			status = L"eating";
			break;
		case Cockroach::Status::FULL:
			status = L"full";
			break;
		case Cockroach::Status::EXPIRED:
			status = L"returning";
			break;
		}
		TextOut(hdc, (int)(m_x + m_radius), (int)(m_y - m_radius), status.c_str(), (int)status.length());
		Ellipse(hdc, (int)(m_x - m_radius), (int)(m_y - m_radius), (int)(m_x + m_radius), (int)(m_y + m_radius));
	}

	void relative_rotate(double angle)
	{
		m_direction = m_direction * std::polar(1.0, angle);
	}

	void rotate(std::complex<double> direction)
	{
		m_direction = direction;
	}

	
	void fright(RECT *rect, double x, double y)
	{
		if (distance(x, y) < SIGHT_RADIUS * 3) {
			double alpha = atan2((m_y - y), (x - m_x));
			m_direction = { sin(alpha - M_PI / 2), cos(alpha - M_PI / 2) };
			m_status = Status::FRIGHTENED;
			m_fright_time = std::chrono::system_clock::now();
		}
	}

	void move(RECT *rect, std::list<std::shared_ptr<Cockroach>> &cockroaches, std::shared_ptr<Cockroach> self, std::list<std::shared_ptr<Food>> &food_drops)
	{
		double sign = rng() % 2 == 0 ? -1.0 : 1.0;

		if (m_x - m_radius < rect->left && m_y - m_radius < rect->top) {
			m_direction = { sin(M_PI / 4), cos(M_PI / 4) };
		}
		else if (m_x + m_radius > rect->right && m_y - m_radius < rect->top) {
			m_direction = { sin(-M_PI / 4), cos(-M_PI / 4) };
		}
		else if (m_x + m_radius > rect->right && m_y + m_radius > rect->bottom) {
			m_direction = { sin(-M_PI * 3 / 4), cos(-M_PI * 3 / 4) };
		}
		else if (m_x - m_radius < rect->left && m_y + m_radius > rect->bottom) {
			m_direction = { sin(M_PI * 3 / 4), cos(M_PI * 3 / 4) };
		}
		else if (m_x - m_radius < rect->left) {
			m_direction = { sin(M_PI / 2), cos(M_PI / 2) };
			relative_rotate(sign * (rng() % 120 * M_PI / 180));
		}
		else if (m_y - m_radius < rect->top) {
			m_direction = { sin(0), cos(0) };
			relative_rotate(sign * (rng() % 120 * M_PI / 180));
		}
		else if (m_x + m_radius > rect->right) {
			m_direction = { sin(-M_PI / 2), cos(-M_PI / 2) };
			relative_rotate(sign * (rng() % 120 * M_PI / 180));
		}
		else if (m_y + m_radius > rect->bottom) {
			m_direction = { sin(-M_PI), cos(-M_PI) };
			relative_rotate(sign * (rng() % 120 * M_PI / 180));
		} 
		else {
			bool was_collision = false;
			for (auto cockroach : cockroaches) {
				if (cockroach == self) { // it's me
					continue;
				}
				if (cockroach->distance(m_x, m_y) < cockroach->get_radius() + m_radius) {
					double alpha = atan2(m_y - cockroach->get_y(), cockroach->get_x() - m_x);
					m_direction = { sin(alpha - M_PI / 2), cos(alpha - M_PI / 2) };

					cockroach->rotate(-1.0 * m_direction);
					was_collision = true;
				}
			}

			if (!was_collision && (m_status == Status::NORMAL || m_status == Status::CONSUMING)) {
				bool food_found = false;
				for (auto food : food_drops) {
					if (distance(food->get_x(), food->get_y()) < SIGHT_RADIUS) {
						food_found = true;
						double alpha = atan2(food->get_y() - m_y, m_x - food->get_x());
						m_direction = { sin(alpha - M_PI / 2), cos(alpha - M_PI / 2) };
						
						if (distance(food->get_x(), food->get_y()) < m_radius + food->get_size()) {
							m_status = Status::CONSUMING;
							food->eat();

							m_radius += Food::PIECE_SIZE;
							if (m_radius >= MAX_RADIUS) {
								m_status = Status::FULL;
							}
						}
						break;
					}
				}
				if (!food_found) {
					m_status = Status::NORMAL;
					relative_rotate(sign * (rng() % 20 * M_PI / 180));
				}
			}

			if (!was_collision && (m_status == Status::FULL || m_status == Status::EXPIRED)) {
				int nearest_corner_x = m_x < rect->right / 2 ? rect->left : rect->right;
				int nearest_corner_y = m_y < rect->bottom / 2 ? rect->top : rect->bottom;

				double alpha = atan2(nearest_corner_y - m_y, m_x - nearest_corner_x);
				m_direction = { sin(alpha - M_PI / 2), cos(alpha - M_PI / 2) };
			}

		}

		double velocity = (m_status == Status::FRIGHTENED ? 2.0 : 1.0) * m_velocity;
		m_x += velocity * m_direction.real();
		m_y += velocity * m_direction.imag();

		if (m_status == Status::FRIGHTENED) {
			auto elapsed_seconds = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - m_fright_time).count();
			if (elapsed_seconds > 4) {
				if (m_radius > MAX_RADIUS) {
					m_status = Status::FULL;
				}
				else {
					m_status = Status::NORMAL;
				}
			}
		}

		if (m_status == Status::NORMAL) {
			auto elapsed_seconds = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - m_spawn_time).count();
			if (elapsed_seconds > 30) {
				m_status = Status::EXPIRED;
			}
		}
	}

	inline double distance(double x, double y)
	{
		return sqrt((m_x - x) * (m_x - x) + (m_y - y) * (m_y - y));
	}

	inline double get_radius() { return m_radius; }
	inline double get_x() {	return m_x; }
	inline double get_y() {	return m_y;	}

	inline Status get_status() { return m_status; }

private:

	std::complex<double> m_direction;
	double m_velocity;
	double m_radius;

	double m_x;
	double m_y;

	Status m_status;
	std::chrono::time_point<std::chrono::system_clock> m_spawn_time;
	std::chrono::time_point<std::chrono::system_clock> m_fright_time;
};

class CockroachFactory
{
public:
	static std::shared_ptr<Cockroach> make_cockroach(int x, int y) {
		return std::make_shared<Cockroach>(x, y);
	}
};
