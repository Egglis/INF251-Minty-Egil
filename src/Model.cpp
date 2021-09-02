#include "Model.h"

#include <list>
#include <fstream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <iterator>
#include <iostream>
#include <limits>
#include <unordered_map>
#include <array>
#include <algorithm> 
#include <cctype>
#include <locale>
#include <filesystem>
#include <globjects/globjects.h>
#include <globjects/logging.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

using namespace minity;
using namespace gl;
using namespace glm;
using namespace globjects;

std::string trimLeft(const std::string &str, const std::string &whitespace = "\n\r\t ")
{
	size_t uIndex = str.find_first_not_of(whitespace);
	if (uIndex != std::string::npos)
		return str.substr(uIndex);

	return "";
}

std::string trimRight(const std::string &str, const std::string &whitespace = "\n\r\t ")
{
	size_t  uIndex = str.find_last_not_of(whitespace);
	if (uIndex != std::string::npos)
		return str.substr(0, uIndex + 1);

	return str;
}

std::string trim(const std::string &str, const std::string & whitespace = "\n\r\t ")
{
	return trimRight(trimLeft(str, whitespace), whitespace);
}

template<class e, class t, int N>
std::basic_istream<e, t>& operator>>(std::basic_istream<e, t>& in, const e(&sliteral)[N])
{
	std::array<e, N - 1> buffer; //get buffer
	in >> buffer[0]; //skips whitespace
	if (N > 2)
		in.read(&buffer[1], N - 2); //read the rest
	if (strncmp(&buffer[0], sliteral, N - 1)) //if it failed
		in.setstate(in.rdstate() | std::ios::failbit); //set the state
	return in;
}

template<class e, class t>
std::basic_istream<e, t>& operator>>(std::basic_istream<e, t>& in, const e& cliteral)
{
	e buffer;  //get buffer
	in >> buffer; //read data
	if (buffer != cliteral) //if it failed
		in.setstate(in.rdstate() | std::ios::failbit); //set the state
	return in;
}

//redirect mutable char arrays to their normal function
template<class e, class t, int N>
std::basic_istream<e, t>& operator>>(std::basic_istream<e, t>& in, e(&carray)[N])
{
	return std::operator>>(in, carray);
}

class ObjLoader
{
public:

	struct ObjGroup
	{
		std::string name;
		std::string material;
		std::vector<uint> positionIndices;
		std::vector<uint> normalIndices;
		std::vector<uint> texCoordIndices;
	};

	struct ObjMaterial
	{
		// Material Name
		std::string name;
		// Ambient Color
		vec3 Ka = vec3(0.2f,0.2f,0.2f);
		// Diffuse Color
		vec3 Kd = vec3(0.8f,0.8f,0.8f);
		// Specular Color
		vec3 Ks = vec3(1.0f,1.0f,1.0f);
		// Specular Exponent
		float Ns = 0.0f;
		// Dissolve
		float d = 1.0f;
		// Illumination
		int illum = 0;
		// Ambient Texture Map
		std::string map_Ka;
		// Diffuse Texture Map
		std::string map_Kd;
		// Specular Texture Map
		std::string map_Ks;
		// Specular Hightlight Map
		std::string map_Ns;
		// Alpha Texture Map
		std::string map_d;
		// Bump Map
		std::string map_bump;
	};

	bool loadObjFile(const std::string & filename)
	{
		std::filesystem::path path(filename);
		std::ifstream is(filename);

		if (!is.is_open())
			return false;

		std::vector< vec3 > positions;
		std::vector< vec3 > normals;
		std::vector< vec2 > texCoords;

		positions.push_back(vec3(0.0f));
		normals.push_back(vec3(0.0f));
		texCoords.push_back(vec2(1.0f));

		std::unordered_map< std::string, int > materialMap;
		std::vector<ObjMaterial> materials;

		ObjMaterial defaultMaterial;
		defaultMaterial.name = "default";

		materialMap.insert(std::make_pair(defaultMaterial.name, int(materials.size())));
		materials.push_back(defaultMaterial);

		std::string currentMaterial;
		currentMaterial = defaultMaterial.name;

		std::unordered_map< std::string, typename std::list<ObjGroup>::iterator > groupMap;
		std::list<ObjGroup> groupList;
		std::list<ObjGroup>::iterator groupIterator;

		ObjGroup defaultGroup;
		defaultGroup.name = "default";
		defaultGroup.material = currentMaterial;
		groupList.push_back(defaultGroup);
		groupIterator = groupList.end();
		groupIterator--;
		groupMap[defaultGroup.name] = groupIterator;

		std::string buffer;

		while (is.good())
		{
			if (getline(is, buffer))
			{
				std::istringstream iss(buffer);
				std::string token;

				if (iss >> token)
				{
					switch (token.at(0))
					{
						// v, vn, vt
					case 'v':
					{
						if (token == "v")
						{
							vec3 p(0.0f);

							if (iss >> p.x >> p.y >> p.z)
								positions.push_back(p);
						}
						else if (token == "vn")
						{
							vec3 n(0.0f);

							if (iss >> n.x >> n.y >> n.z)
								normals.push_back(n);
						}
						else if (token == "vt")
						{
							vec2 t(0.0f);

							if (iss >> t.x >> t.y)
								texCoords.push_back(t);
						}
					}
					break;

					// mtllib
					case 'm':
					{
						// the Wavefront obj specification does not really allow for spaces in the mtl file name,
						// since multiple libraries are supposed to be separated by spaces, but many programs
						// do not take care of that -- therefore, we first try whether it is a single filename,
						// and only if that fails we use the interpretation according to the specification
						std::string libraryName;

						if (getline(iss, libraryName))
						{
							libraryName = trim(libraryName);
							std::filesystem::path libraryPath = libraryName;

							// first try
							if (libraryPath.is_absolute())
							{
								if (loadMtlFile(libraryPath.string(), materials, materialMap))
									break;
							}

							std::stringstream mss(libraryName);

							while (mss >> libraryName)
							{
								libraryName = trim(libraryName);
								std::filesystem::path libraryPath = path.parent_path();
								libraryPath.append(libraryName);
								loadMtlFile(libraryPath.string(), materials, materialMap);
							}
						}
					}
					break;

					// use material
					case 'u':
					{
						std::string materialName;

						if (getline(iss, materialName))
						{
							materialName = trim(materialName);
							currentMaterial = materialName;
							groupIterator->material = currentMaterial;
						}
					}
					break;

					// group
					case 'g':
					case 'o':
					{
						std::string groupName;
						
						if (getline(iss,groupName))
						{
							groupName = trim(groupName);
							std::unordered_map< std::string, typename std::list<ObjGroup>::iterator >::iterator j = groupMap.find(groupName);

							if (j == groupMap.end())
							{
								ObjGroup newGroup;
								newGroup.name = groupName;

								groupList.push_back(newGroup);
								groupIterator = groupList.end();
								groupIterator--;
							}
							else
								groupIterator = j->second;

							groupIterator->material = currentMaterial;
						}
					}
					break;

					// face
					case 'f':
					{
						int v = 0, n = 0, t = 0;

						if (buffer.find("//") != std::string::npos)
						{
							// v//n
							if (iss >> v >> ("//") >> n)
							{
								groupIterator->positionIndices.push_back(v < 0 ? (uint) (v + positions.size()) : (uint) (v));
								groupIterator->texCoordIndices.push_back(0);
								groupIterator->normalIndices.push_back(n < 0 ? (uint) (n + normals.size()) : (uint) (n));
							}

							if (iss >> v >> ("//") >> n)
							{
								groupIterator->positionIndices.push_back(v < 0 ? (uint) (v + positions.size()) : (uint) (v));
								groupIterator->texCoordIndices.push_back(0);
								groupIterator->normalIndices.push_back(n < 0 ? (uint) (n + normals.size()) : (uint) (n));
							}

							if (iss >> v >> ("//") >> n)
							{
								groupIterator->positionIndices.push_back(v < 0 ? (uint) (v + positions.size()) : (uint) (v));
								groupIterator->texCoordIndices.push_back(0);
								groupIterator->normalIndices.push_back(n < 0 ? (uint) (n + normals.size()) : (uint) (n));
							}

							while (iss >> v >> ("//") >> n)
							{
								groupIterator->positionIndices.push_back(groupIterator->positionIndices[groupIterator->positionIndices.size() - 3]);
								groupIterator->texCoordIndices.push_back(0);
								groupIterator->normalIndices.push_back(groupIterator->normalIndices[groupIterator->normalIndices.size() - 3]);

								groupIterator->positionIndices.push_back(groupIterator->positionIndices[groupIterator->positionIndices.size() - 2]);
								groupIterator->texCoordIndices.push_back(0);
								groupIterator->normalIndices.push_back(groupIterator->normalIndices[groupIterator->normalIndices.size() - 2]);

								groupIterator->positionIndices.push_back(v < 0 ? (uint) (v + positions.size()) : (uint) (v));
								groupIterator->texCoordIndices.push_back(0);
								groupIterator->normalIndices.push_back(n < 0 ? (uint) (n + normals.size()) : (uint) (n));
							}

							break;
						}

						iss = std::istringstream(buffer);
						iss >> token;

						if (iss >> v >> ("/") >> t >> ("/") >> n)
						{
							// v/t/n
							groupIterator->positionIndices.push_back(v < 0 ? (uint) (v + positions.size()) : (uint) (v));
							groupIterator->texCoordIndices.push_back(t < 0 ? (uint) (t + texCoords.size()) : (uint) (t));
							groupIterator->normalIndices.push_back(n < 0 ? (uint) (n + normals.size()) : (uint) (n));

							if (iss >> v >> ("/") >> t >> ("/") >> n)
							{
								groupIterator->positionIndices.push_back(v < 0 ? (uint) (v + positions.size()) : (uint) (v));
								groupIterator->texCoordIndices.push_back(t < 0 ? (uint) (t + texCoords.size()) : (uint) (t));
								groupIterator->normalIndices.push_back(n < 0 ? (uint) (n + normals.size()) : (uint) (n));
							}

							if (iss >> v >> ("/") >> t >> ("/") >> n)
							{
								groupIterator->positionIndices.push_back(v < 0 ? (uint) (v + positions.size()) : (uint) (v));
								groupIterator->texCoordIndices.push_back(t < 0 ? (uint) (t + texCoords.size()) : (uint) (t));
								groupIterator->normalIndices.push_back(n < 0 ? (uint) (n + normals.size()) : (uint) (n));
							}

							while (iss >> v >> ("/") >> t >> ("/") >> n)
							{
								groupIterator->positionIndices.push_back(groupIterator->positionIndices[groupIterator->positionIndices.size() - 3]);
								groupIterator->texCoordIndices.push_back(groupIterator->texCoordIndices[groupIterator->texCoordIndices.size() - 3]);
								groupIterator->normalIndices.push_back(groupIterator->normalIndices[groupIterator->normalIndices.size() - 3]);

								groupIterator->positionIndices.push_back(groupIterator->positionIndices[groupIterator->positionIndices.size() - 2]);
								groupIterator->texCoordIndices.push_back(groupIterator->texCoordIndices[groupIterator->texCoordIndices.size() - 2]);
								groupIterator->normalIndices.push_back(groupIterator->normalIndices[groupIterator->normalIndices.size() - 2]);

								groupIterator->positionIndices.push_back(v < 0 ? (uint) (v + positions.size()) : (uint) (v));
								groupIterator->texCoordIndices.push_back(t < 0 ? (uint) (t + texCoords.size()) : (uint) (t));
								groupIterator->normalIndices.push_back(n < 0 ? (uint) (n + normals.size()) : (uint) (n));
							}

							break;
						}

						iss = std::istringstream(buffer);
						iss >> token;

						if (iss >> v >> ("/") >> t)
						{
							// v/t
							groupIterator->positionIndices.push_back(v < 0 ? (uint) (v + positions.size()) : (uint) (v));
							groupIterator->texCoordIndices.push_back(t < 0 ? (uint) (t + texCoords.size()) : (uint) (t));
							groupIterator->normalIndices.push_back(0);

							if (iss >> v >> ("/") >> t)
							{
								groupIterator->positionIndices.push_back(v < 0 ? (uint) (v + positions.size()) : (uint) (v));
								groupIterator->texCoordIndices.push_back(t < 0 ? (uint) (t + texCoords.size()) : (uint) (t));
								groupIterator->normalIndices.push_back(0);
							}

							if (iss >> v >> ("/") >> t)
							{
								groupIterator->positionIndices.push_back(v < 0 ? (uint) (v + positions.size()) : (uint) (v));
								groupIterator->texCoordIndices.push_back(t < 0 ? (uint) (t + texCoords.size()) : (uint) (t));
								groupIterator->normalIndices.push_back(0);
							}

							while (iss >> v >> ("/") >> t)
							{
								groupIterator->positionIndices.push_back(groupIterator->positionIndices[groupIterator->positionIndices.size() - 3]);
								groupIterator->texCoordIndices.push_back(groupIterator->texCoordIndices[groupIterator->texCoordIndices.size() - 3]);
								groupIterator->normalIndices.push_back(0);

								groupIterator->positionIndices.push_back(groupIterator->positionIndices[groupIterator->positionIndices.size() - 2]);
								groupIterator->texCoordIndices.push_back(groupIterator->texCoordIndices[groupIterator->texCoordIndices.size() - 2]);
								groupIterator->normalIndices.push_back(0);

								groupIterator->positionIndices.push_back(v < 0 ? (uint) (v + positions.size()) : (uint) (v));
								groupIterator->texCoordIndices.push_back(t < 0 ? (uint) (t + texCoords.size()) : (uint) (t));
								groupIterator->normalIndices.push_back(0);
							}

							break;
						}
						else
						{
							iss = std::istringstream(buffer);
							iss >> token;

							// v
							if (iss >> v)
							{
								groupIterator->positionIndices.push_back(v < 0 ? (uint) (v + positions.size()) : (uint) (v));
								groupIterator->texCoordIndices.push_back(0);
								groupIterator->normalIndices.push_back(0);
							}

							if (iss >> v)
							{
								groupIterator->positionIndices.push_back(v < 0 ? (uint) (v + positions.size()) : (uint) (v));
								groupIterator->texCoordIndices.push_back(0);
								groupIterator->normalIndices.push_back(0);
							}

							if (iss >> v)
							{
								groupIterator->positionIndices.push_back(v < 0 ? (uint) (v + positions.size()) : (uint) (v));
								groupIterator->texCoordIndices.push_back(0);
								groupIterator->normalIndices.push_back(0);
							}

							while (iss >> v)
							{
								groupIterator->positionIndices.push_back(groupIterator->positionIndices[groupIterator->positionIndices.size() - 3]);
								groupIterator->texCoordIndices.push_back(0);
								groupIterator->normalIndices.push_back(0);

								groupIterator->positionIndices.push_back(groupIterator->positionIndices[groupIterator->positionIndices.size() - 2]);
								groupIterator->texCoordIndices.push_back(0);
								groupIterator->normalIndices.push_back(0);

								groupIterator->positionIndices.push_back(v < 0 ? (uint) (v + positions.size()) : (uint) (v));
								groupIterator->texCoordIndices.push_back(0);
								groupIterator->normalIndices.push_back(0);
							}
						}
					}
					break;
					}
				}
			}
		}

		if (materials.size() <= 1)
		{
			std::filesystem::path libraryPath = path;
			libraryPath.replace_extension("mtl");
			loadMtlFile(libraryPath.string(), materials, materialMap);
		}

		// compute normals if not present in the file
		if (normals.size() <= 1)
		{
			// compute face normals
			for (std::list<ObjGroup>::iterator i = groupList.begin(); i != groupList.end(); i++)
			{
				if (i->positionIndices.size() > 0)
				{
					for (uint j = 0; j < i->positionIndices.size() - 2; j += 3)
					{
						const vec3 & p0 = positions[i->positionIndices[j + 0]];
						const vec3 & p1 = positions[i->positionIndices[j + 1]];
						const vec3 & p2 = positions[i->positionIndices[j + 2]];

						if (i->normalIndices[j + 0] == 0 || i->normalIndices[j + 1] == 0 || i->normalIndices[j + 2] == 0)
						{
							if (i->normalIndices[j + 0] == 0)
								i->normalIndices[j + 0] = uint(normals.size());

							if (i->normalIndices[j + 1] == 0)
								i->normalIndices[j + 1] = uint(normals.size());

							if (i->normalIndices[j + 2] == 0)
								i->normalIndices[j + 2] = uint(normals.size());

							const vec3 a(p2 - p1);
							const vec3 b(p0 - p1);
							const vec3 n = normalize(cross(a, b));
							normals.push_back(n);
						}
					}
				}
			}

			// compute vertex normals
			std::vector< vec3 > vertexNormals(positions.size());
			std::vector< vec3 > groupNormals(positions.size());

			for (std::list<ObjGroup>::iterator i = groupList.begin(); i != groupList.end(); i++)
			{
				if (i->positionIndices.size() > 0)
				{
					for (uint j = 0; j < i->positionIndices.size() - 2; j += 3)
					{
						groupNormals[i->positionIndices[j + 0]] = groupNormals[i->positionIndices[j + 0]] + normals[i->normalIndices[j + 0]];
						groupNormals[i->positionIndices[j + 1]] = groupNormals[i->positionIndices[j + 1]] + normals[i->normalIndices[j + 1]];
						groupNormals[i->positionIndices[j + 2]] = groupNormals[i->positionIndices[j + 2]] + normals[i->normalIndices[j + 2]];
					}

					for (uint j = 0; j < i->positionIndices.size() - 2; j += 3)
					{
						vertexNormals[i->positionIndices[j + 0]] = normalize(groupNormals[i->positionIndices[j + 0]]);
						vertexNormals[i->positionIndices[j + 1]] = normalize(groupNormals[i->positionIndices[j + 1]]);
						vertexNormals[i->positionIndices[j + 2]] = normalize(groupNormals[i->positionIndices[j + 2]]);
					}

					for (uint j = 0; j < i->positionIndices.size() - 2; j += 3)
					{
						groupNormals[i->positionIndices[j + 0]] = vec3(0.0f);
						groupNormals[i->positionIndices[j + 1]] = vec3(0.0f);
						groupNormals[i->positionIndices[j + 2]] = vec3(0.0f);
					}

					i->normalIndices = i->positionIndices;
				}
			}

			normals.swap(vertexNormals);
		}

		m_vertices.resize(positions.size());

		for (std::list<ObjGroup>::iterator i = groupList.begin(); i != groupList.end(); i++)
		{
			if (i->positionIndices.size() > 0)
			{
				Group newGroup;
				newGroup.name = i->name;
				newGroup.startIndex = uint(m_indices.size());

				std::unordered_map<std::string, int>::iterator j = materialMap.find(i->material);

				if (j != materialMap.end())
					newGroup.materialIndex = j->second;
				else
					newGroup.materialIndex = 0;

				for (uint j = 0; j < i->positionIndices.size(); j++)
				{
					const uint index = i->positionIndices[j];

					Vertex vertex;
					vertex.position = positions[index];
					vertex.normal = normals[i->normalIndices[j]];
					vertex.texcoord = texCoords[i->texCoordIndices[j]];

					if (m_vertices[index].position == vertex.position)
					{
						if (m_vertices[index].texcoord != vertex.texcoord || m_vertices[index].normal != vertex.normal)
						{
							m_indices.push_back(uint(m_vertices.size()));
							m_vertices.push_back(vertex);
						}
						else
						{
							m_vertices[index] = vertex;
							m_indices.push_back(index);
						}
					}
					else
					{
						m_vertices[index] = vertex;
						m_indices.push_back(index);
					}
				}

				newGroup.endIndex = uint(m_indices.size());
				m_groups.push_back(newGroup);
			}
		}

		m_materials.reserve(materials.size());
		

		for (auto & m : materials)
		{
			Material newMaterial;
			newMaterial.ambient = m.Ka;
			newMaterial.diffuse = m.Kd;
			newMaterial.specular = m.Ks;
			newMaterial.shininess = m.Ns;

			if (!m.map_Ka.empty())
			{
				std::filesystem::path texturePath = m.map_Ka;

				if (!texturePath.is_absolute())
				{
					texturePath = path.parent_path();
					texturePath.append(m.map_Ka);
				}

				newMaterial.ambientTexture = std::move(loadTexture(texturePath.string()));
			}

			if (!m.map_Kd.empty())
			{
				std::filesystem::path texturePath = m.map_Kd;

				if (!texturePath.is_absolute())
				{
					texturePath = path.parent_path();
					texturePath.append(m.map_Kd);
				}

				newMaterial.diffuseTexture = std::move(loadTexture(texturePath.string()));
			}

			if (!m.map_Ks.empty())
			{
				std::filesystem::path texturePath = m.map_Ks;

				if (!texturePath.is_absolute())
				{
					texturePath = path.parent_path();
					texturePath.append(m.map_Ks);
				}

				newMaterial.specularTexture = std::move(loadTexture(texturePath.string()));
			}

			if (!m.map_Ns.empty())
			{
				std::filesystem::path texturePath = m.map_Ns;

				if (!texturePath.is_absolute())
				{
					texturePath = path.parent_path();
					texturePath.append(m.map_Ns);
				}

				newMaterial.shininessTexture = std::move(loadTexture(texturePath.string()));
			}

			if (!m.map_bump.empty())
			{
				std::filesystem::path texturePath = m.map_bump;

				if (!texturePath.is_absolute())
				{
					texturePath = path.parent_path();
					texturePath.append(m.map_bump);
				}

				newMaterial.bumpTexture = std::move(loadTexture(texturePath.string()));
			}

			m_materials.push_back(newMaterial);

		}

		return true;
	}

	bool loadMtlFile(const std::string & filename, std::vector<ObjMaterial> & materials, std::unordered_map< std::string, int > & materialMap)
	{
		std::ifstream is(filename);

		if (!is.is_open())
			return false;

		std::string buffer;
		int currentMaterialIndex = 0;

		while (is.good())
		{
			if (getline(is, buffer))
			{
				std::istringstream iss(buffer);
				std::string token;

				if (iss >> token)
				{
					if (token == "newmtl")
					{
						std::string materialName;

						if (getline(iss, materialName))
						{
							materialName = trim(materialName);

							auto i = materialMap.find(materialName);

							if (i == materialMap.end())
							{
								ObjMaterial newMaterial;
								newMaterial.name = materialName;
								currentMaterialIndex = uint(materials.size());
								materialMap.insert(std::make_pair(newMaterial.name, currentMaterialIndex));
								materials.push_back(newMaterial);
							}
							else
							{
								currentMaterialIndex = i->second;
							}
						}
					}
					// Ambient Color
					else if (token == "Ka")
					{
						vec3 Ka(0.0f);
						if (iss >> Ka.x >> Ka.y >> Ka.z)
							materials[currentMaterialIndex].Ka = Ka;
					}
					// Diffuse Color
					else if (token == "Kd")
					{
						vec3 Kd(0.0f);
						if (iss >> Kd.x >> Kd.y >> Kd.z)
							materials[currentMaterialIndex].Kd = Kd;
					}
					// Specular Color
					else if (token == "Ks")
					{
						vec3 Ks(0.0f);
						if (iss >> Ks.x >> Ks.y >> Ks.z)
							materials[currentMaterialIndex].Ks = Ks;
					}
					// Specular Exponent
					else if (token == "Ns")
					{
						float Ns = 0.0f;
						if (iss >> Ns)
							materials[currentMaterialIndex].Ns = Ns;
					}
					// Dissolve
					else if (token == "d")
					{
						float d = 0.0f;
						if (iss >> d)
							materials[currentMaterialIndex].d = d;
					}
					// Illumination
					else if (token == "illum")
					{
						int illum = 0;
						if (iss >> illum)
							materials[currentMaterialIndex].illum = illum;
					}
					// Ambient Texture Map
					else if (token == "map_Ka")
					{
						std::string map_Ka;
						if (getline(iss, map_Ka))
						{
							map_Ka = trim(map_Ka);
							materials[currentMaterialIndex].map_Ka = map_Ka;
						}
					}
					// Diffuse Texture Map
					else if (token == "map_Kd")
					{
						std::string map_Kd;
						if (getline(iss, map_Kd))
						{
							map_Kd = trim(map_Kd);
							materials[currentMaterialIndex].map_Kd = map_Kd;
						}
					}
					// Specular Texture Map
					else if (token == "map_Ks")
					{
						std::string map_Ks;
						if (getline(iss, map_Ks))
						{
							map_Ks = trim(map_Ks);
							materials[currentMaterialIndex].map_Ks = map_Ks;
						}
					}
					// Specular Hightlight Map
					else if (token == "map_Ns")
					{
						std::string map_Ns;
						if (getline(iss, map_Ns))
						{
							map_Ns = trim(map_Ns);
							materials[currentMaterialIndex].map_Ns = map_Ns;
						}
					}
					// Alpha Texture Map
					else if (token == "map_d")
					{
						std::string map_d;
						if (getline(iss, map_d))
						{
							map_d = trim(map_d);
							materials[currentMaterialIndex].map_d = map_d;
						}
					}
					// Bump Map
					else if (token == "map_bump" || buffer == "map_Bump" || buffer == "bump")
					{
						std::string map_bump;
						if (getline(iss, map_bump))
						{
							map_bump = trim(map_bump);
							materials[currentMaterialIndex].map_bump = map_bump;
						}
					}
				}
			}
		}

		return true;
	}

	std::unique_ptr<Texture> loadTexture(const std::string & filename)
	{
		int width, height, channels;

		stbi_set_flip_vertically_on_load(true);
		unsigned char *data = stbi_load(filename.c_str(), &width, &height, &channels, 0);

		if (data)
		{
			std::cout << "Loaded " << filename << std::endl;

			auto texture = Texture::create(GL_TEXTURE_2D);
			texture->setParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			texture->setParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			texture->setParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
			texture->setParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);

			GLenum format = GL_RGBA;

			switch (channels)
			{
			case 1:
				format = GL_RED;
				break;

			case 2:
				format = GL_RG;
				break;

			case 3:
				format = GL_RGB;
				break;

			case 4:
				format = GL_RGBA;
				break;
			}
				
			texture->image2D(0, format, ivec2(width,height), 0, format, GL_UNSIGNED_BYTE, data);
			texture->generateMipmap();

			stbi_image_free(data);

			return texture;
		}

		return std::unique_ptr<Texture>();
	}

	const std::vector<Group> & groups() const
	{
		return m_groups;
	}

	const std::vector<Vertex> & vertices() const
	{
		return m_vertices;
	}

	const std::vector<uint> & indices() const
	{
		return m_indices;
	}

	const std::vector<Material> & materials() const
	{
		return m_materials;
	}

private:

	std::vector < Group > m_groups;
	std::vector < Vertex > m_vertices;
	std::vector < glm::uint > m_indices;
	std::vector < Material > m_materials;

};

Model::Model()
{

}

Model::Model(const std::string& filename)
{
	load(filename);
}

void Model::load(const std::string& filename)
{
	globjects::debug() << "Loading file " << filename << " ...";

	m_minimumBounds = vec3(std::numeric_limits<float>::max());
	m_maximumBounds = vec3(-std::numeric_limits<float>::max());

	ObjLoader loader;

	if (loader.loadObjFile(filename))
	{
		m_filename = filename;
		m_vertices = loader.vertices();
		m_indices = loader.indices();
		m_materials = loader.materials();
		m_groups = loader.groups();

		for (auto i : m_indices)
		{
			const auto & v = m_vertices[i];
			m_minimumBounds = min(m_minimumBounds, v.position);
			m_maximumBounds = max(m_maximumBounds, v.position);
		}

		globjects::debug() << "Minimum bounds: " << m_minimumBounds;
		globjects::debug() << "Maximum bounds: " << m_maximumBounds;

		m_vertexBuffer->setStorage(m_vertices, gl::GL_NONE_BIT);
		m_indexBuffer->setStorage(m_indices, gl::GL_NONE_BIT);

		auto vertexBindingPosition = m_vertexArray->binding(0);
		vertexBindingPosition->setAttribute(0);
		vertexBindingPosition->setBuffer(m_vertexBuffer.get(), 0, sizeof(Vertex));
		vertexBindingPosition->setFormat(3, GL_FLOAT);
		m_vertexArray->enable(0);

		auto vertexBindingNormal = m_vertexArray->binding(1);
		vertexBindingNormal->setAttribute(1);
		vertexBindingNormal->setBuffer(m_vertexBuffer.get(), sizeof(vec3), sizeof(Vertex));
		vertexBindingNormal->setFormat(3, GL_FLOAT);
		m_vertexArray->enable(1);

		auto vertexBindingTexCoord = m_vertexArray->binding(2);
		vertexBindingTexCoord->setAttribute(2);
		vertexBindingTexCoord->setBuffer(m_vertexBuffer.get(), sizeof(vec3) + sizeof(vec3), sizeof(Vertex));
		vertexBindingTexCoord->setFormat(2, GL_FLOAT);
		m_vertexArray->enable(2);

		m_vertexArray->bindElementBuffer(m_indexBuffer.get());

	}
	else
	{
		globjects::debug() << "Error loading << " << filename << "!";
	}
}

const std::string & Model::filename() const
{
	return m_filename;
}

const std::vector<Group> & Model::groups() const
{
	return m_groups;
}

const std::vector<Vertex> & Model::vertices() const
{
	return m_vertices;
}

const std::vector<uint> & Model::indices() const
{
	return m_indices;
}

const std::vector<Material> & Model::materials() const
{
	return m_materials;
}

vec3 Model::minimumBounds() const
{
	return m_minimumBounds;
}

vec3 Model::maximumBounds() const
{
	return m_maximumBounds;
}

VertexArray & Model::vertexArray()
{
	return *m_vertexArray.get();
}

Buffer & Model::vertexBuffer()
{
	return *m_vertexBuffer.get();
}

Buffer & Model::indexBuffer()
{
	return *m_indexBuffer.get();
}

