#pragma once
#include <iostream>
#include <vector>
#include <curl/curl.h>
#include <rapidjson/document.h>
#include <opencv2/opencv.hpp>

#include "Base64.hpp"

static size_t CurlWriteCallbackVectorBuffer(void* contents, size_t _size, size_t nmemb, void* userp) {
	size_t size = _size * nmemb;

	std::vector<uint8_t>& vec = *(std::vector<uint8_t>*)userp;

	size_t old_size = vec.size();

	vec.resize(vec.size() + size);

	memcpy(&vec[old_size], contents, size);

	return size;
}

static size_t CurlWriteCallbackString(void* contents, size_t size, size_t nmemb, void* userp)
{
	((std::string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}

static size_t CurlWriteCallbackOSTREAM(void* buf, size_t size, size_t nmemb, void* userp)
{
	std::ostream& os = *static_cast<std::ostream*>(userp);
	std::streamsize len = size * nmemb;
	if (os.write(static_cast<char*>(buf), len))
		return len;

	return 0;
}

void CurlDownloadToOSTREAM(const std::string& url, std::ostream& os)
{
	CURL* easyhandle = curl_easy_init();

	curl_easy_setopt(easyhandle, CURLOPT_URL, url.c_str());

	curl_easy_setopt(easyhandle, CURLOPT_WRITEFUNCTION, &CurlWriteCallbackOSTREAM);

	curl_easy_setopt(easyhandle, CURLOPT_WRITEDATA, &os);

	curl_easy_perform(easyhandle);

	curl_easy_cleanup(easyhandle);

}

cv::Mat CurlDownloadToMat(const std::string& url)
{
	CURL* easyhandle = curl_easy_init();

	std::vector<unsigned char> vec;

	curl_easy_setopt(easyhandle, CURLOPT_URL, url.c_str());

	curl_easy_setopt(easyhandle, CURLOPT_WRITEFUNCTION, &CurlWriteCallbackVectorBuffer);

	curl_easy_setopt(easyhandle, CURLOPT_WRITEDATA, &vec);

	curl_easy_perform(easyhandle);

	curl_easy_cleanup(easyhandle);

	return cv::imdecode(vec, cv::IMREAD_UNCHANGED);
}

std::string CurlDownloadToString(const std::string& url) {

	std::string ret;

	CURL* easyhandle = curl_easy_init();

	curl_easy_setopt(easyhandle, CURLOPT_URL, url.c_str());

	curl_easy_setopt(easyhandle, CURLOPT_WRITEFUNCTION, CurlWriteCallbackString);

	curl_easy_setopt(easyhandle, CURLOPT_WRITEDATA, &ret);

	curl_easy_perform(easyhandle);

	curl_easy_cleanup(easyhandle);

	return ret;
}

// Returns true if the skin uses the Alex playermodel, and false if the skin uses the Steve playermodel.
bool GetModel(const std::string& username) {
	std::string res = CurlDownloadToString("https://api.mojang.com/users/profiles/minecraft/" + username);

	rapidjson::Document document;
	document.Parse(res.c_str());

	res = CurlDownloadToString("https://sessionserver.mojang.com/session/minecraft/profile/" + (std::string)(document["id"].GetString()));

	document.Parse(res.c_str());
	res = base64_decode(document["properties"][0]["value"].GetString());

	document.Parse(res.c_str());

	if (document["textures"]["SKIN"]["metadata"].IsObject())
		return (std::string)document["textures"]["SKIN"]["metadata"]["model"].GetString() == "slim";
	else
		return false;
}

cv::Mat DownloadSkin(const std::string& username) {
	std::string res = CurlDownloadToString("https://api.mojang.com/users/profiles/minecraft/" + username);

	rapidjson::Document document;
	document.Parse(res.c_str());

	res = CurlDownloadToString("https://sessionserver.mojang.com/session/minecraft/profile/" + (std::string)(document["id"].GetString()));

	document.Parse(res.c_str());
	res = base64_decode(document["properties"][0]["value"].GetString());

	document.Parse(res.c_str());

	std::string URL = document["textures"]["SKIN"]["url"].GetString();

	return CurlDownloadToMat(URL);
}

cv::Mat ConvertToModernSkin(cv::Mat& skin, bool model) {
	if (skin.rows == skin.cols)
		return skin;

	cv::Mat ret(64, 64, CV_8UC4);
	skin.copyTo(ret(cv::Rect(0, 0, 64, 32)));
    
    if (model) { // Alex
		cv::flip(skin(cv::Rect(40, 20, 4, 12)), ret(cv::Rect(39, 52, 4, 12)), 1); // ArmSide1
		cv::flip(skin(cv::Rect(44, 20, 3, 12)), ret(cv::Rect(36, 52, 3, 12)), 1); // ArmSide2
		cv::flip(skin(cv::Rect(47, 20, 4, 12)), ret(cv::Rect(32, 52, 4, 12)), 1); // ArmSide3
		cv::flip(skin(cv::Rect(44, 20, 3, 12)), ret(cv::Rect(43, 52, 3, 12)), 1); // ArmSide4
		cv::flip(skin(cv::Rect(44, 16, 3, 4)), ret(cv::Rect(36, 48, 3, 4)), 1); // ArmBTM
		cv::flip(skin(cv::Rect(47, 16, 3, 4)), ret(cv::Rect(39, 48, 3, 4)), 1); // ArmTOP

	}
    else {
		cv::flip(skin(cv::Rect(40, 20, 4, 12)), ret(cv::Rect(40, 52, 4, 12)), 1); // ArmSide1
		cv::flip(skin(cv::Rect(44, 20, 4, 12)), ret(cv::Rect(36, 52, 4, 12)), 1); // ArmSide2
		cv::flip(skin(cv::Rect(48, 20, 4, 12)), ret(cv::Rect(32, 52, 4, 12)), 1); // ArmSide3
		cv::flip(skin(cv::Rect(52, 20, 4, 12)), ret(cv::Rect(44, 52, 4, 12)), 1); // ArmSide4
		cv::flip(skin(cv::Rect(44, 16, 4, 4)), ret(cv::Rect(36, 48, 4, 4)), 1); // ArmBTM
		cv::flip(skin(cv::Rect(48, 16, 4, 4)), ret(cv::Rect(40, 48, 4, 4)), 1); // ArmTOP
	}

	cv::flip(skin(cv::Rect(0, 20, 4, 12)), ret(cv::Rect(24, 52, 4, 12)), 1); // LegSide1
	cv::flip(skin(cv::Rect(4, 20, 4, 12)), ret(cv::Rect(20, 52, 4, 12)), 1); // LegSide2
	cv::flip(skin(cv::Rect(8, 20, 4, 12)), ret(cv::Rect(16, 52, 4, 12)), 1); // LegSide3
	cv::flip(skin(cv::Rect(12, 20, 4, 12)), ret(cv::Rect(28, 52, 4, 12)), 1); // LegSide4
	cv::flip(skin(cv::Rect(4, 16, 4, 4)), ret(cv::Rect(20, 48, 4, 4)), 1); // LegBTM
	cv::flip(skin(cv::Rect(8, 16, 4, 4)), ret(cv::Rect(24, 48, 4, 4)), 1); // LegTOP

	return ret;
}

void ColorCorrectSkin(cv::Mat& skin) {
	for (int y = 0; y < skin.rows; ++y) {
		for (int x = 0; x < skin.cols; ++x) {

			RGBA& rgba = *(RGBA*)&skin.data[y * skin.step + 4 * x];

			rgba.R = (powf((float)rgba.R / 255.0f, 1.0f / 1.2f)) * 191.0f;
			rgba.G = (powf((float)rgba.G / 255.0f, 1.0f / 1.2f)) * 191.0f;
			rgba.B = (powf((float)rgba.B / 255.0f, 1.0f / 1.2f)) * 191.0f;
		}
	}
}