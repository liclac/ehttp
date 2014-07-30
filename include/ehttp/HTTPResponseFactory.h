#ifndef EHTTP_HTTPRESPONSEFACTORY_H
#define EHTTP_HTTPRESPONSEFACTORY_H

#include "HTTPResponse.h"
#include "HTTPRequest.h"

#include <memory>

namespace ehttp
{
	/**
	 * A class that automates creation of HTTPResponse objects.
	 * 
	 * 
	 */
	template<typename... Args>
	class HTTPResponseFactory
	{
	public:
		/**
		 * Constructs a response from the given request.
		 * 
		 * If #onDataFunc or #onEndFunc are available, they will be used to
		 * construct HTTPResponse::onData and HTTPResponse::onEnd respectively,
		 * falling back to #onData and #onEnd.
		 */
		virtual std::shared_ptr<HTTPResponse> res(std::shared_ptr<HTTPRequest> req, Args&&... args)
		{
			auto _onData = (onDataFunc ? onDataFunc(std::forward<Args>(args)...) : onData);
			auto _onEnd = (onEndFunc ? onEndFunc(std::forward<Args>(args)...) : onEnd);
			return std::make_shared<HTTPResponse>(req, _onData, _onEnd);
		}
		
		
		
		/// Value for HTTPResponse::onData of constructed responses
		std::function<void(std::shared_ptr<HTTPResponse> res, std::vector<char> data)> onData;
		/// Value for HTTPResponse::onEnd of constructed responses
		std::function<void(std::shared_ptr<HTTPResponse> res)> onEnd;
		
		/// Function that constructs HTTPResponse::onData
		std::function<std::function<void(std::shared_ptr<HTTPResponse> res, std::vector<char> data)>(Args&&...)> onDataFunc;
		/// Function that constructs HTTPResponse::onEnd
		std::function<std::function<void(std::shared_ptr<HTTPResponse> res)>(Args&&...)> onEndFunc;
	};
}

#endif
