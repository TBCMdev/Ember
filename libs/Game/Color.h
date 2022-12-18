#pragma once
namespace marine {
	struct Color {
		float red, green, blue, alpha;
		static Color rgba(float r, float g, float b, float a = 255) {
			return Color{ r,g,b,a };
		}
	};
}