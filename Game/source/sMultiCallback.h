#pragma once

template <typename T>
struct sMultiCallback
{
private:
	std::vector<std::function<T>> boundCallbacks;

public:
	// Adds inDelegate to the callback functor vector
	void bind(const std::function<T>& inDelegate)
	{
		boundCallbacks.emplace_back(inDelegate);
	}

	// Calls all functors added to the callback functor vector
	template<typename... args>
	void broadcast(args&&... inArgs)
	{
		for (const std::function<T>& function : boundCallbacks)
		{
			function(std::forward<args>(inArgs)...);
		}
	}

	// Clears all functors from the callback functor vector
	void clear()
	{
		boundCallbacks.clear();
	}
};