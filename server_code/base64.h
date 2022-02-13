#ifndef __Base_64_h__
#define __Base_64_h__
#include <cv.h>
#include <string>
#include <vector>

static const std::string base64_chars =
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";
static inline bool is_base64(unsigned char c) {
	  return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string base64_encode(std::vector<unsigned char> buffer);
std::vector<unsigned char> base64_decode(std::string const& encoded_string);
static bool MatToBase64(cv::Mat input, std::string ext,
			std::string &output, std::vector<int> params=std::vector<int>());
static bool Base64ToMat(std::string input, cv::Mat & output,
		std::string &ext);


#endif