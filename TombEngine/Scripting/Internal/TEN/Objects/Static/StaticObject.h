#pragma once

#include "Objects/NamedBase.h"
#include "Game/room.h"

namespace sol {
	class state;
}

class Vec3;
class Rotation;
class ScriptColor;

class Static : public NamedBase<Static, MESH_INFO &>
{
public:
	using IdentifierType = std::reference_wrapper<MESH_INFO>;
	Static(MESH_INFO & id);
	~Static() = default;

	Static& operator=(Static const& other) = delete;
	Static(Static const& other) = delete;

	static void Register(sol::table & parent);

	void Enable();
	void Disable();
	bool GetActive();
	bool GetSolid();
	void SetSolid(bool yes);
	Rotation GetRot() const;
	void SetRot(Rotation const& rot);
	Vec3 GetPos() const;
	void SetPos(Vec3 const & pos);
	float GetScale() const;
	void SetScale(float const& scale);
	std::string GetName() const;
	void SetName(std::string const & name);
	int GetSlot() const;
	void SetSlot(int slot);
	ScriptColor GetColor() const;
	void SetColor(ScriptColor const & col);
	void Shatter();

private:
	MESH_INFO & m_mesh;
};
