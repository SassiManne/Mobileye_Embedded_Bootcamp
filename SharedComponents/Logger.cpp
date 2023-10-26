
#include "Logger.h"
std::unique_ptr<Logger> Logger::infoLog = std::make_unique<Logger>("Info");
std::unique_ptr<Logger> Logger::errorLog = std::make_unique<Logger>("Error");

Logger& Logger::getInfoInstance() {
	if (infoLog != nullptr)
		return *infoLog.get();
}

Logger& Logger::getErrorInstance() {
	if (errorLog != nullptr)
		return *errorLog.get();
}

void Logger::setPattern(const std::string& pattern) {
	this->logger_->set_pattern(pattern);
}

void Logger::logInfo(const std::string& message) {
	this->logger_->info(message);
	this->logger_->flush();
}

void Logger::logError(const std::string& message) {
	this->logger_->error(message);
	this->logger_->flush();
}

void Logger::logWarn(const std::string& message) {
	this->logger_->warn(message);
	this->logger_->flush();
}

void Logger::logDebug(const std::string& message) {
	this->logger_->debug(message);
	this->logger_->flush();
}

void Logger::shutdown() {
	this->logger_->flush();
	//this->logger_->flush_on(spdlog::level::info);
}

void Logger::Init() {

	
		infoLog->setPattern("[%H:%M:%S %z] [%^%l%$] [thread %t] %v");
		errorLog->setPattern("[%H:%M:%S %z] [%^%l%$] [thread %t] %v");

}

