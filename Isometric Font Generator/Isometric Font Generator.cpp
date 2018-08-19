#include "stdafx.h"
#include "Isometric Font Generator.h"

const std::string fontFolder = "C:\\Users\\David White\\Documents\\Rainmeter\\Skins\\IsometricBase\\@Resources\\Fonts";

int main() {
	// Open files for output
	std::ofstream topFile(fontFolder + "\\fontTop.bf", std::ios::trunc);
	std::ofstream shadowFile(fontFolder + "\\fontShadow.bf", std::ios::trunc);
	std::ofstream sideFile(fontFolder + "\\fontSide.bf", std::ios::trunc);
	std::ofstream frontFile(fontFolder + "\\fontFront.bf", std::ios::trunc);

	// Open data file, and check for success
	INIReader reader("data.ini");
	if(reader.ParseError() < 0) {
		std::cout << "Can't load 'data.ini'\n";
	} else {
		// Get list of sections, and check that [HeaderData] is present
		std::set<std::string> sections = reader.Sections();
		std::set<std::string>::iterator headerData;
		if((headerData = sections.find("HeaderData")) == sections.end()) {
			std::cout << "Error: No [HeaderData] section found.";
		} else {
			// INPROG: Create headers
			writeHeaders(topFile, shadowFile, sideFile, frontFile, reader);

			// TEMP
			int width = 5;
			int height = 10;
			std::vector<std::vector<bool>> glyph(width + 2, std::vector<bool>(height + 2));
			glyph[5][1] = true;
			glyph[2][2] = true;
			glyph[3][2] = true;
			glyph[4][2] = true;
			// TEMP

			// TODO: Iterate through each glyph
			do {
				// TODO: Write unicode ID
				// TODO: Write margins
				// Generate top font paths
				string topPaths;
				topPaths = generateTop(width, height, glyph);
				std::cout << topPaths;
				// TODO: Write top font paths to top file
				// TODO: Generate shadow font paths
				// TODO: Write shadow font paths to shadow file
				// TODO: Generate side font paths
				// TODO: Write side font paths to side file
				// TODO: Generate front font paths
				// TODO: Write front font paths to front file
			} while(false);
			// TODO: Create special cases for .notdef and ¶
			// TODO: Write footers
		}
	}
	string end;
	std::cout << "Press enter to continue...";
	std::cin >> end;
	return 0;
}

void writeHeaders(std::ofstream &topFile, std::ofstream &shadowFile, std::ofstream &sideFile, std::ofstream &frontFile, INIReader &reader) {
	string temp = "<?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\"?>\n<font>\n<format svg=\"false\">3.4</format>\n\n<postscript_name>";
	topFile << temp; shadowFile << temp; sideFile << temp; frontFile << temp;
	topFile << reader.Get("HeaderData", "top_font_name", "IsometricTop"); shadowFile << reader.Get("HeaderData", "shadow_font_name", "IsometricShadow"); sideFile << reader.Get("HeaderData", "side_font_name", "IsometricSide"); frontFile << reader.Get("HeaderData", "front_font_name", "IsometricFront");
}


// NOTE: The matrix dimensions are NOT 0-indexed, as there is a buffer ring around the outside.
string generateTop(int width, int height, std::vector<std::vector<bool>> glyph) {
	string pathData = "";
	for(int y = 1; y <= height; y++) {
		for(int x = 1; x <= width; x++) {
			if(glyph[x][y] && !glyph[x][y - 1] && (glyph[x - 1][y - 1] == glyph[x - 1][y])) {
				int length = 1;
				while(glyph[x + 1][y] && !glyph[x + 1][y - 1]) {
					length++;
					x++;
				}
				x++;
				pathData += "\n\t\t\t<path data=\"S " + convertCoords(x - length, y, 0.0) + " L " + convertCoords(x, y, 0.0) + " L " + (glyph[x][y - 1] ? convertCoords(x + 0.5, y - 0.5, 1.0) + " L " : "") + convertCoords(x, y, 1.0) + " L " + convertCoords(x - length, y, 1.0) + " L " + convertCoords(x - length, y, 0.0) + "\" />";
			}
		}
	}
	return pathData;
}

// NOTE: This function takes xin and yin with an origin of 1,1 and converts them to a 0,0 origin, due to the buffer layer aroung each glyph array.
string convertCoords(double xin, double yin, double zin) {
	xin--;
	yin--;
	xin += zin;
	yin += zin;
	std::stringstream ss;
	ss << std::fixed << std::setprecision(10) << 9.0 * ((2 * xin - yin) / sqrt(3)) << ',' << 9.0 * -yin;
	return ss.str();
}
