#include "saved_view.h"

#include <chrono>

// * static

std::vector<SavedView> SavedView::allViews;

#ifdef NDEBUG
constexpr const char* INI_FILE_PATH = "saved_views.ini";
#else
constexpr const char* INI_FILE_PATH = "../bin/saved_views.ini";
#endif

IniFile<int> SavedView::iniFile{INI_FILE_PATH};

void SavedView::initFromFile() {
	for (const auto& pair : iniFile) {
		//allViews.insert(std::cbegin(allViews), std::move(SavedView{pair.first, pair.second})); // Add to front
		allViews.push_back(SavedView{pair.first, pair.second});
	}
}

void SavedView::saveNew(long double zoomScale, const ComplexNum& startNum, const std::string& name) {
	allViews.push_back(SavedView{zoomScale, startNum, name});
	const SavedView& currentElement = *(allViews.cend() - 1);
	iniFile.set(currentElement.imGuiIDs[0], currentElement.viewDataToString());
}

void SavedView::removeSavedView(const SavedView& savedView) {
	iniFile.remove(savedView.imGuiIDs[0]);
	auto iterator = std::find(allViews.cbegin(), allViews.cend(), savedView);
	allViews.erase(iterator);
}


// * non-static

void SavedView::setName(const std::string& name) {
	this->name = name;
	replaceAll(this->name, "|", "");
	iniFile.set(imGuiIDs[0], viewDataToString());
}

bool SavedView::isIDValid(int id) {
	if (id == -1)
		return false; // -1 is invalid

	for (const SavedView& savedView : allViews) {
		for (int c = 0; c < NUMBER_OF_IDS; c++) {
			if (id == savedView.imGuiIDs[c])
				return false;
		}
	}
	return true;
}

int SavedView::createNewID() const {
    std::srand(std::chrono::high_resolution_clock::now().time_since_epoch().count());
	int newID;
	do {
		newID = hashDjb2(getName()) + std::rand();
	} 
	while (!isIDValid(newID));
	return newID;
}

SavedView::SavedView(long double zoomScale, const ComplexNum& startNum, const std::string& name)
    : zoomScale(zoomScale), startNum(startNum), name(name)
{
    if (name.empty())
        this->name = createGenericName();

	for (int& imGuiID : imGuiIDs)
		imGuiID = createNewID();
}

SavedView::SavedView(int firstID, const std::string& viewData) {
	// Format: "name|zoomScale|startNum.first|startNum.second|imGuiIds[1]|...|imGuiIds['last']|"
	std::string value;
	std::stringstream stream{viewData};

	imGuiIDs[0] = firstID;

	std::getline(stream, name, '|');

	std::getline(stream, value, '|');
	zoomScale = std::stold(value);

	std::getline(stream, value, '|');
	startNum.first = std::stold(value);

	std::getline(stream, value, '|');
	startNum.second = std::stold(value);

	for (int i = 0; std::getline(stream, value, '|'); i++)
		imGuiIDs[i + 1] = std::stoi(value);
}

std::string SavedView::createGenericName() const {
    std::stringstream stream;
    stream.precision(5);
    stream << startNum.first + 0.5L * zoomScale << " + " << startNum.second + 0.5L * zoomScale << " i (" << zoomScale << ")";
    return stream.str();
}

std::string SavedView::viewDataToString() const {
	// Format: "name|zoomScale|startNum.first|startNum.second|imGuiIds[1]|...|imGuiIds['last']|"
	std::stringstream stream;
	stream.precision(std::numeric_limits<long double>::max_digits10);
	stream << name << '|' << zoomScale << '|' << startNum.first << '|' << startNum.second << '|';
	for (int i = 1; i < imGuiIDs.size(); i++)
		stream << imGuiIDs[i] << '|';
	return stream.str();
}
