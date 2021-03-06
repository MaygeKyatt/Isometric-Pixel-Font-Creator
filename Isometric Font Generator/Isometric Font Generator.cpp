#include "stdafx.h"
#include "Isometric Font Generator.h"

int main() {
	// Open data file, and check for success
	INIReader reader("data.ini");
	if(reader.ParseError() < 0) {
		std::cout << "Can't load 'data.ini'\n";
	} else {
		// Get list of sections
		std::set<std::string> sections = reader.Sections();

		// Check that [ProgramData] is present
		if(sections.find("ProgramData") == sections.end()) {
			std::cout << "Error: No [ProgramData] section found.";
		} else {
			std::string fontFolder = reader.Get("ProgramData", "file_path", "");

			// Check that [HeaderData] is present
			if(sections.find("HeaderData") == sections.end()) {
				std::cout << "Error: No [HeaderData] section found.";
			} else {
				// Open files for output
				std::ofstream files[4];
				files[0].open(fontFolder + "\\fontTop.bf", std::ios::trunc);
				files[1].open(fontFolder + "\\fontShadow.bf", std::ios::trunc);
				files[2].open(fontFolder + "\\fontSide.bf", std::ios::trunc);
				files[3].open(fontFolder + "\\fontFront.bf", std::ios::trunc);

				// Create headers
				writeHeaders(files, reader);

				// Iterate through each glyph
				for(std::string id : sections) {
					if((id != "HeaderData") && (id != "ProgramData")) {
						// Find glyph width & height (in pixels)
						int width = reader.GetInteger(id, "width", 1);
						int height = reader.GetInteger("ProgramData", "height", 1);

						// Read pixel matrix
						std::vector<std::vector<bool>> glyph(width + 2, std::vector<bool>(height + 2));
						std::size_t pos = 0;
						std::string glyphString = reader.Get(id, "glyph", "");
						for(int y = 1; y <= height; y++) {
							for(int x = 1; x <= width; x++) {
								if((pos < glyphString.length()) && (glyphString[pos] == '1'))
									glyph[x][y] = true;
								pos++;
							}
						}

						// Write unicode id, margins
						if(id == ".notdef") {
							writeToAll(files, "\n\n<collection name=\".notdef");
						} else {
							writeToAll(files, "\n\n<collection unicode=\"U+" + id);
						}
						writeToAll(files, "\" svg=\"false\" auto_generate=\"false\">\n\t<selected id=\"0\"/>\n\t<glyph id=\"0\" left=\"0\" right=\"" + std::to_string(9 * (2 * (reader.GetInteger(id, "width", 0) + 2) / sqrt(3))) + "\">\n\t\t<layer name= \"Layer\" visible=\"true\">");

						// Generate and write top font paths
						files[0] << generateTop(width, height, glyph);
						// INPROG: Generate and write shadow font paths

						// Generate and write side font paths
						files[2] << generateSide(width, height, glyph);

						// Generate and write front font paths
						files[3] << generateFront(width, height, glyph);

						// Close glyph tags
						writeToAll(files, "\n\t\t</layer>\n\t</glyph>\n</collection>");
					}
				}
				// Close <font> tag
				writeToAll(files, "\n\n\n\n\n</font>");
			}
		}
	}
	return 0;
}

std::string generateTop(int width, int height, std::vector<std::vector<bool>> &glyph) {
	std::string pathData = "";
	for(int y = 1; y <= height; y++) {
		for(int x = 1; x <= width; x++) {
			if(glyph[x][y] && !glyph[x][y - 1] && !(!glyph[x - 1][y - 1] && glyph[x - 1][y])) {
				int length = 1;
				while(glyph[x + 1][y] && !glyph[x + 1][y - 1]) {
					length++;
					x++;
				}
				x++;
				pathData += "\n\t\t\t<path data=\"S " + convertCoords(x - length, y, 0.0) + " L " + convertCoords(x, y, 0.0) + " L " + (glyph[x][y - 1] ? convertCoords(x + 0.5, y - 0.5, 1.0) + " L " : "") + convertCoords(x, y, 1.0) + " L " + convertCoords(x - length, y + 0.5, 1.0) + " L " + convertCoords(x - length, y, 0.0) + "\" />";
			}
		}
	}
	return pathData;
}

std::string generateShadow(int width, int height, std::vector<std::vector<bool>>& glyph, INIReader & reader) {
	std::string pathData = "";
	double shadowAngle = convertToRads(reader.GetReal("ProgramData", "shadow_angle", 107.0));
	double shadowLength = reader.GetReal("ProgramData", "shadow_length", 1.5);
	double shadowWidth = shadowLength * cos(shadowAngle);
	double shadowHeight = shadowLength * sin(shadowAngle);

	// Generate lists of starting points (2 potential types)
	std::vector<coord> startingPointsV, startingPointsH;
	for(int y = 1; y <= height; y++) {
		for(int x = 1; x <= width; x++) {
			if(!glyph[x][y] && glyph[x - 1][y] && glyph[x][y - 1])
				startingPointsH.emplace_back(x, y);
			else if(glyph[x][y] && !glyph[x - 1][y] && !glyph[x - 1][y - 1] && !glyph[x][y - 1])
				startingPointsV.emplace_back(x, y);
		}
	}

	while(startingPointsH.empty() || startingPointsV.empty()) {
		// true == down, false == right
		bool dir, looping = true;
		std::string thisPath = "";
		int xPos, yPos;
		if(!startingPointsV.empty()) {
			coord current = startingPointsV.back();
			xPos = startingPointsV[0].x;
			yPos = startingPointsV[0].y;
			startingPointsV.pop_back();
			if(glyph[xPos - 1][yPos + 1] && shadowHeight > 1.0) {
				double ratio = 1 / shadowHeight;
				thisPath += "\n\t\t\t<path data=\"S " + convertCoords(xPos, yPos, 0.0) + " L " + convertCoords(xPos - ratio * shadowWidth, yPos + 1, 0.0) + " L " + convertCoords(xPos - ratio * shadowWidth, yPos + 1, 1.0 - ratio) + " L " + convertCoords(xPos, yPos + 1, 1.0) + " L " + convertCoords(xPos, yPos, 0.0);
				looping = false;
			}
		} else {
			coord current = startingPointsH.back();
			xPos = startingPointsH[0].x;
			yPos = startingPointsH[0].y;
			startingPointsH.pop_back();
			dir = false;
			thisPath = convertCoords(xPos + 0.5, yPos - 0.5, 0.0) + " L " + convertCoords(xPos, yPos, 0.0) + " L " + convertCoords(xPos, yPos + shadowHeight, 0.0);
		}

		while(looping) {

			// TODO: check if shadow lands on lower pixels
		}
	}

	return pathData;
}

std::string generateSide(int width, int height, std::vector<std::vector<bool>> &glyph) {
	std::string pathData = "";
	for(int x = 1; x <= width; x++) {
		for(int y = 1; y <= height; y++) {
			if(glyph[x][y] && !glyph[x - 1][y] && !(!glyph[x - 1][y - 1] && glyph[x][y - 1])) {
				int length = 1;
				while(glyph[x][y + 1] && !glyph[x - 1][y + 1]) {
					length++;
					y++;
				}
				y++;
				pathData += "\n\t\t\t<path data=\"S " + convertCoords(x, y - length, 0.0) + " L " + convertCoords(x, y, 0.0) + " L " + convertCoords(x, y, 1.0) + " L " + convertCoords(x, y - length, 1.5) + " L " + convertCoords(x, y - length, 0.0) + "\" />";
			}
		}
	}
	return pathData;
}

std::string generateFront(int width, int height, std::vector<std::vector<bool>>& glyph) {
	std::string pathData = "";

	// Generate list of potential starting points
	std::set<coord, CoordCompare> startingPoints;
	for(int y = 1; y <= height; y++) {
		for(int x = 1; x <= width; x++) {
			if(!glyph[x - 1][y - 1] && (glyph[x][y - 1] || glyph[x][y]) && (glyph[x - 1][y] == glyph[x][y - 1]))
				startingPoints.emplace(x, y);
		}
	}

	// Generate paths from starting points
	while(!startingPoints.empty()) {
		int xStart = startingPoints.begin()->x; int yStart = startingPoints.begin()->y; int xPos = xStart; int yPos = yStart;
		bool isFirstPoint = true;
		pathData += "\n\t\t\t<path data=\"S " + convertCoords(xPos, yPos, 1.0);
		unsigned int dirM = (glyph[xPos][yPos - 1] ? 3000 : 3001);
		do {
			unsigned int dir = dirM % 4;
			// Erase current pos as possibility
			if(!((dir == 3) && !glyph[xPos - 1][yPos - 1]))
				startingPoints.erase(coord(xPos, yPos));

			// Turn path if neccessary
			std::array<std::array<int, 2>, 5> poi = {{{-1,-1}, {0,-1}, {0,0}, {-1,0}, {-1,-1}}};
			if(glyph[xPos + poi[dir][0]][yPos + poi[dir][1]]) {
				dirM--;
				pathData += " L " + convertCoords(xPos, yPos, 1.0);
				isFirstPoint = false;
			} else {
				if(!glyph[xPos + poi[dir + 1][0]][yPos + poi[dir + 1][1]]) {
					dirM++;
					pathData += " L " + convertCoords(xPos, yPos, 1.0);
					isFirstPoint = false;
				}
			}

			// Extend path
			switch(dirM % 4) {
			case 0:
				yPos--;
				break;
			case 1:
				xPos++;
				break;
			case 2:
				yPos++;
				break;
			case 3:
				xPos--;
				break;
			}
		} while(!((xPos == xStart) && (yPos == yStart)));
		// Close path
		pathData += " L " + convertCoords(xPos, yPos, 1.0) + "\" />";
	}
	return pathData;
}

double convertToRads(double degrees) {
	return degrees * M_PI / 180.0;
}

// NOTE: This function takes xin and yin with an origin of 1,1 and converts them to a 0,0 origin, due to the buffer layer around each glyph array.
std::string convertCoords(double xin, double yin, double zin) {
	xin--; yin--; xin += zin; yin += zin;
	std::stringstream ss;
	ss << std::fixed << std::setprecision(10) << 9.0 * ((2 * xin - yin) / sqrt(3)) << ',' << 9.0 * -yin;
	return ss.str();
}

void writeHeaders(std::ofstream files[], INIReader & reader) {
	writeToAll(files, "<?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\"?>\n<font>\n<format svg=\"false\">3.4</format>\n");

	writeNameTag("postscript_name", files, reader);
	writeNameTag("name", files, reader);

	std::vector<tagIdentifier> tags = {
		{"subfamily", "Regular"},
		{"bold", "false"},
		{"italic", "false"}};
	writeTagList(tags, reader, files);

	writeNameTag("full_name", files, reader);
	writeNameTag("unique_identifier", files, reader);

	tags = {
		{"version", "Regular"},
		{"description", "false"},
		{"copyright", "SIL Open Font License (OFL)"},
		{"license", "SIL Open Font License (OFL)"},
		{"license_url", "http://scripts.sil.org/OFL"},
		{"weight", "400"},
		{"units_per_em", "1024"},
		{"trademark", ""},
		{"manufacturer", ""},
		{"designer", ""},
		{"vendor_url", ""},
		{"designer_url", ""}};
	writeTagList(tags, reader, files);

	writeToAll(files, "\n\n<horizontal>\n\t<top_limit>0.0000000000</top_limit>\n\t<top_position>0.0000000000</top_position>\n\t<x-height>-27.0000000000</x-height>\n\t<base_line>-72.0000000000</base_line>\n\t<bottom_position>-90.0000000000</bottom_position>\n\t<bottom_limit>-99.0000000000</bottom_limit>\n</horizontal>\n\n<grid width=\"9.0000\"/>\n<grid width=\"1.0000\"/>\n<grid width=\"2.0000\"/>\n<grid width=\"1.0000\"/>\n<grid width=\"2.0000\"/>\n<grid width=\"4.0000\"/>\n<background scale=\"1\" />");
}

void writeTagList(std::vector<tagIdentifier> &tags, INIReader & reader, std::ofstream * files) {
	for(tagIdentifier tag : tags) {
		writeToAll(files, "\n<" + tag.tagName + ">" + reader.Get("HeaderData", tag.tagName, tag.defaultVal) + "</" + tag.tagName + ">");
	}
}

void writeNameTag(std::string tag, std::ofstream * files, INIReader & reader) {
	writeToAll(files, "\n<" + tag + ">");
	files[0] << reader.Get("HeaderData", "top_font_name", "IsometricTop"); files[1] << reader.Get("HeaderData", "shadow_font_name", "IsometricShadow"); files[2] << reader.Get("HeaderData", "side_font_name", "IsometricSide"); files[3] << reader.Get("HeaderData", "front_font_name", "IsometricFront");
	writeToAll(files, "</" + tag + ">");
}

void writeToAll(std::ofstream * files, std::string temp) {
	files[0] << temp; files[1] << temp; files[2] << temp; files[3] << temp;
}
