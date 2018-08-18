#include "stdafx.h"
#include "Isometric Font Generator.h"

int main() {
	// TODO: Delete old files
	// TODO: Read data file, store data
	int width = 5;
	int height = 10;
	std::vector<std::vector<bool>> glyph(width + 2, std::vector<bool>(height + 2));
	glyph[5][1] = true;
	glyph[2][2] = true;
	glyph[3][2] = true;
	glyph[4][2] = true;
	// TODO: Create new files
		// TODO: Create headers
	// TODO: Iterate through each glyph
	do {
		// TODO: Write unicode ID
		// INPROG: Generate top font paths
		generateTop(width, height, glyph);
		// TODO: Write top font paths to top file
		// TODO: Generate shadow font paths
		// TODO: Write shadow font paths to shadow file
		// TODO: Generate side font paths
		// TODO: Write side font paths to side file
		// TODO: Generate front font paths
		// TODO: Write front font paths to front file
	} while(false);
	// TODO: Write footers

	system("pause");
	return 0;
}

// NOTE: The matrix dimensions are NOT 0-indexed, as there is a buffer ring around the outside.
std::string generateTop(int width, int height, std::vector<std::vector<bool>> glyph) {
	std::string pathData = "";
	for(int y = 1; y <= height; y++) {
		for(int x = 1; x <= width; x++) {
			if(glyph[x][y] && !glyph[x][y - 1] && (glyph[x - 1][y - 1] == glyph[x - 1][y])) {
				int length = 1;
				while(glyph[x + 1][y] && !glyph[x + 1][y - 1]) {
					length++;
					x++;
				};
				x++;
				pathData += "\n\t\t\t<path data=\"S " + convertCoords(x - length, y, 0.0) + " L " + convertCoords(x, y, 0.0) + " L " + (glyph[x][y - 1] ? convertCoords(x + 0.5, y - 0.5, 1.0) + " L " : "") + convertCoords(x, y, 1.0) + " L " + convertCoords(x - length, y, 1.0) + " L " + convertCoords(x - length, y, 0.0) + "\" />";
			}
		}
	}
	return pathData;
}

// NOTE: This function takes xin and yin with an origin of 1,1 and converts them to a 0,0 origin, due to the buffer layer aroung each glyph array.
std::string convertCoords(double xin, double yin, double zin) {
	xin--;
	yin--;
	xin += zin;
	yin += zin;
	std::stringstream ss;
	// TODO: Fix mult on xin
	ss << std::fixed << std::setprecision(10) << 9.0 * ((2 * xin - yin) / sqrt(3)) << ',' << 9.0 * -yin;
	return ss.str();
}
