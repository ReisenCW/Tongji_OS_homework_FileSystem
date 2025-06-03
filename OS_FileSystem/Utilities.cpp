#include "Utilities.h"
#include <sstream>
#include <vector>

// 定义并初始化全局的 config 变量
Config config;

std::string getFullPath(const std::string& relativePath) {
    std::string actualPath;
    if (relativePath.empty()) {
        actualPath = config.realRootPath + "/" + config.currentPath.substr(config.rootPath.length());
    }
    else if (relativePath[0] == '/') {
        actualPath = config.realRootPath + relativePath;
    }
    else {
        std::string currentActualPath = config.realRootPath + config.currentPath.substr(config.rootPath.length());
        if (!currentActualPath.empty() && currentActualPath.back() != '/') {
            currentActualPath += '/';
        }
        actualPath = currentActualPath + relativePath;
    }
    // 确保路径不以 '/' 结尾
    if (!actualPath.empty() && actualPath.back() == '/') {
        actualPath.pop_back();
    }
    return actualPath;
}

std::string simplifyPath(const std::string & path) {
    std::vector<std::string> stack;
    std::istringstream iss(path);
    std::string part;

    while (std::getline(iss, part, '/')) {
        if (part == "..") {
            if (!stack.empty()) {
                stack.pop_back();
            }
        }
        else if (!part.empty() && part != ".") {
            stack.push_back(part);
        }
    }

    std::string simplifiedPath;
    for (const auto& p : stack) {
        simplifiedPath += "/" + p;
    }

    return simplifiedPath.empty() ? "/" : simplifiedPath;
}