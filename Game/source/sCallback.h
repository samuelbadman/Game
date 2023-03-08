#pragma once

template <typename T>
struct sCallback
{
private:
	std::function<T> boundCallback = nullptr;

public:
	// Binds the delegate functor to the callback
	void bind(const std::function<T>& inDelegate) { boundCallback = inDelegate; }

	// Attempts to call the callback functor without checking if the a functor is bound. Causes a crash if called without a bound callback functor
	template<typename... args>
	void call(args&&... inArgs)
	{
		boundCallback(std::forward<args>(inArgs)...);
	}

	// First checks if a functor is bound to the callback before calling the callback functor. Safe to call if callback functor is unbound
	template<typename... args>
	void callIfBound(args&&... inArgs)
	{
		if (isBound())
		{
			boundCallback(std::forward<args>(inArgs)...);
		}
	}

	// Unbinds the callback functor from a bound function
	void unbind() { boundCallback = nullptr; }

	// Returns true if a function is bound to the callback functor and is safe to call
	bool isBound() const { return boundCallback != nullptr; }
};
