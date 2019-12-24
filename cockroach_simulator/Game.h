#pragma once

#include <Windows.h>
#include <algorithm>
#include <array>
#include <chrono>
#include <list>
#include <memory>
#include <optional>
#include <string>
#include <utility>
//#include <mutex>

#include "Cockroach.h"
#include "Food.h"
#include "static_global_vars.h"

class Game
{
public:
	Game(HWND hWnd) : m_hWnd(hWnd)
	{
		m_backbuffDC = CreateCompatibleDC(GetDC(m_hWnd));
		m_memBM		 = CreateCompatibleBitmap(GetDC(m_hWnd), 1920, 1080);

		SelectObject(m_backbuffDC, m_memBM);
		SelectObject(m_backbuffDC, m_hFont);
		m_last_food_drop_time = std::chrono::system_clock::now();
	}

	~Game()
	{
		DeleteObject(m_memBM);
		DeleteObject(m_hFont);
		DeleteDC(m_backbuffDC);
	}

	void start_game()
	{
		DWORD ThreadID;
		m_IsRunning		= true;
		m_thread_handle = CreateThread(NULL, 0, static_thread_start, (void *)this, 0, &ThreadID);
	}

	void stop_game()
	{
		m_IsRunning = false;
		WaitForSingleObject(m_thread_handle, INFINITE);
	}

	inline bool isUpdated()
	{
		return m_isUpdated;
	}
	inline void update()
	{
		m_isUpdated = true;
	}
	inline HDC getBackbuffDC()
	{
		return m_backbuffDC;
	}

private:
	static DWORD WINAPI static_thread_start(void *Param)
	{
		Game *This = (Game *)Param;
		return This->thread_start();
	}

	DWORD thread_start(void)
	{
		while (tick())
		{
			Sleep(50);
		}
		return 0;
	}

	std::optional<std::pair<int, int>> drop_food(RECT *rect)
	{
		if (m_food_drops.size() < 3)
		{
			int x = (int)((rng() % (rect->right - rect->left - (int)Food::DEFAULT_SIZE / 2)) + Food::DEFAULT_SIZE / 2);
			int y = (int)((rng() % (rect->bottom - rect->top - (int)Food::DEFAULT_SIZE / 2)) + Food::DEFAULT_SIZE / 2);

			m_food_drops.push_back(FoodFactory::make_food(x, y));
			return {std::pair(x, y)};
		}
		return {};
	}

	bool spawn(RECT *rect)
	{
		static std::array<int, 4> corners {0, 1, 2, 3};

		if (m_cockroaches.size() < 10)
		{

			std::shuffle(corners.begin(), corners.end(), rng);

			bool free_top_left	   = true;
			bool free_top_right	   = true;
			bool free_bottom_right = true;
			bool free_bottom_left  = true;

			for (auto cockroach : m_cockroaches)
			{
				double cockroach_radius = cockroach->get_radius();
				if (cockroach->distance(rect->left, rect->top) < cockroach_radius * 2)
				{
					free_top_left = false;
				}
				if (cockroach->distance(rect->right, rect->top) < cockroach_radius * 2)
				{
					free_top_right = false;
				}
				if (cockroach->distance(rect->right, rect->bottom) < cockroach_radius * 2)
				{
					free_bottom_right = false;
				}
				if (cockroach->distance(rect->left, rect->bottom) < cockroach_radius * 2)
				{
					free_bottom_left = false;
				}
			}

			for (int i : corners)
			{
				switch (i)
				{
				case 0:
					if (free_top_left)
					{
						m_cockroaches.push_back(CockroachFactory::make_cockroach(rect->left, rect->top));
						return true;
					}
				case 1:
					if (free_top_right)
					{
						m_cockroaches.push_back(CockroachFactory::make_cockroach(rect->right, rect->top));
						return true;
					}
				case 2:
					if (free_bottom_right)
					{
						m_cockroaches.push_back(CockroachFactory::make_cockroach(rect->right, rect->bottom));
						return true;
					}
				case 3:
					if (free_bottom_left)
					{
						m_cockroaches.push_back(CockroachFactory::make_cockroach(rect->left, rect->bottom));
						return true;
					}
				}
			}
		}
		return false;
	}

	bool tick()
	{
		if (!m_IsRunning)
		{
			return false;
		}
		// m_drawMutex.lock();

		HDC	 hdc = GetDC(m_hWnd);
		RECT rect;
		GetClientRect(m_hWnd, &rect);
		SelectObject(m_backbuffDC, m_memBM);

		FillRect(m_backbuffDC, &rect, (HBRUSH)(COLOR_WINDOWFRAME));

		if (rng() % 50 == 0)
		{
			spawn(&rect);
		}

		auto elapsed_seconds =
			std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - m_last_food_drop_time)
				.count();
		if (elapsed_seconds > 20)
		{
			auto drop_coordinates = drop_food(&rect);
			if (drop_coordinates)
			{
				m_last_food_drop_time = std::chrono::system_clock::now();
				for (auto cockroach : m_cockroaches)
				{
					cockroach->fright(&rect, drop_coordinates->first, drop_coordinates->second);
				}
			}
		}

		m_food_drops.remove_if([this](auto food) {
			if (food->get_size() <= 0.0)
			{
				m_last_food_drop_time = std::chrono::system_clock::now();
				return true;
			}
			return false;
		});

		m_cockroaches.remove_if([&rect](auto cockroach) {
			if ((cockroach->get_status() == Cockroach::Status::FULL ||
					cockroach->get_status() == Cockroach::Status::EXPIRED) &&
				(cockroach->distance(rect.left, rect.top) <= cockroach->get_radius() * 1.7 ||
					cockroach->distance(rect.right, rect.top) <= cockroach->get_radius() * 1.7 ||
					cockroach->distance(rect.left, rect.bottom) <= cockroach->get_radius() * 1.7 ||
					cockroach->distance(rect.right, rect.bottom) <= cockroach->get_radius() * 1.7))
			{
				return true;
			}
			return false;
		});

		for (auto food : m_food_drops)
		{
			food->draw(m_backbuffDC, m_cockroaches);
		}

		for (auto cockroach : m_cockroaches)
		{
			cockroach->move(&rect, m_cockroaches, cockroach, m_food_drops);
			cockroach->draw(m_backbuffDC);
		}

		m_isUpdated = false;
		ReleaseDC(m_hWnd, hdc);
		InvalidateRect(m_hWnd, &rect, false);
		UpdateWindow(m_hWnd);

		// m_drawMutex.unlock();
		return true;
	}

	bool m_IsRunning;
	bool m_isUpdated;

	HANDLE	m_thread_handle;
	HWND	m_hWnd;
	HDC		m_backbuffDC;
	HBITMAP m_memBM;
	HFONT	m_hFont = CreateFont(12, 0, 0, 0, FW_LIGHT, 0, 0, 0, 0, 0, 0, 2, 0, L"Segoe UI");

	// std::mutex m_drawMutex;

	std::chrono::time_point<std::chrono::system_clock> m_last_food_drop_time;
	std::list<std::shared_ptr<Cockroach>>			   m_cockroaches;
	std::list<std::shared_ptr<Food>>				   m_food_drops;
};
