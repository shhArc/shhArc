-- DummyNode1b Specializes DummyNode

function shhInitialize(parameters)
	Vector(parameters["p1"], parameters["p1"], parameters["p1"])
end


function shhUpdate(delta)
	a = shh.ThisNode():ReadInput("dave", 1)
	Vector(a,a,a)
	shh.ThisNode():WriteOutput("dave", 1, a+1)
end

function shhFinalize()
	--shh.YieldProcess()
	Vector(1000,1000,1000)
end

