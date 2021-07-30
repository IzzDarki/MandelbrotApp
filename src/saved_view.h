#pragma once
#ifndef MANDELBROT_SAVEDVIEW_INCLUDED
#define MANDELBROT_SAVEDVIEW_INCLUDED

#include <vector>
#include <array>
#include <sstream>
#include <string>
#include <compare>

#include "app_utility.h"
#include "ini_file.h"

class SavedView {

// * static
public:
    static constexpr auto NUMBER_OF_IDS = 3;
    using IDsList_t = std::array<int, NUMBER_OF_IDS>;
    static std::vector<SavedView> allViews;
    static void initFromFile();
    static void saveNew(long double zoomScale, const ComplexNum& startNum, const std::string& name = "");
    static void removeSavedView(const SavedView& savedView);
    
protected:
    static IniFile<int> iniFile;
    static bool isIDValid(int id);

// * non-static
protected:
	long double zoomScale;
	ComplexNum startNum;
    std::string name;
	IDsList_t imGuiIDs; // -1 is an invalid id

public:
    inline long double getZoomScale() const { return zoomScale; }
    inline const ComplexNum& getStartNum() const { return startNum; }
    inline const std::string& getName() const { return name; }
    inline const IDsList_t& getImGuiIDs() const { return imGuiIDs; }

    void setName(const std::string& name);

    auto operator<=>(const SavedView& other) const = default;

protected:
    SavedView(long double zoomScale, const ComplexNum& startNum, const std::string& name = "");
    SavedView(int firstID, const std::string& viewData);
    std::string createGenericName() const;
    int createNewID() const;
    std::string viewDataToString() const;

};

#endif
