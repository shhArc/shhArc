-- DummyAgent1b Specializes DummyAgent

function shhInitialize()
	ok, node = Node("DemoHardNode", {}, {}, {})
	shh.ExpressSchema("test", "Creature")
	shh.Trace("% %", ok, node)
	return Vector(-20,-20,-20)
end


function shhFinalize()
	Vector(-30,-30,-30)
end

function shhMessageDave(vec)
	Dummy___shhMessageDave(vec)
end

function shhUpdate(delta)
	Vector(-40,-40,-40)
end