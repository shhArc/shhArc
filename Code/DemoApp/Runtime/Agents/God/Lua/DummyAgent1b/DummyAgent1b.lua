-- DummyAgent1b Specializes DummyAgent

function shhInitialize()
	ok, node = Node("DemoHardNode", {}, {}, {})
	shh.SendMsg(node, 0.0, "", "Dave", 666)
	shh.ExpressSchema("test", "Creature")
	shh.Trace("% %", ok, node)
	return Vector(-30,-30,-30)
end


function shhFinalize()
	Vector(-50,-50,-50)
end

function shhMessageDave(vec)
	Dummy___shhMessageDave(vec)
end

function shhUpdate(delta)
	Vector(-40,-40,-40)
end