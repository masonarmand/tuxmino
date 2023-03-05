--[[
--
--
--
--]]

--[[ This Function gets called once upon loading the game --]]
function start()
    -- Initialization and backgrounds
    initGameMode("TuxMaster", 255, 255, 255) -- Name and frame color (r, g, b)
    initBackgrounds(1) -- Number of backgrounds
    initBgIdx(0, "res/bg/tuxmaster/00.png")
    setBackground(0)

    setDAS(12)
    setARR(1)
    setARE(15)
    setLineARE(0)
    setLockDelay(30)
    setClearSpeed(10)
    setGravity(4)
    setPreview(3)
    setBagRetry(6)
    setGradeType(1)
    setGrade(0)
    setSevenBag(false)
    setHold(true)
    setSonicDrop(true)
    setExtraKicks(true)
    set3D(true)
    setDrawNextBg(true)
    setBig(false)
    setInvis(false)
    setClearOnCredits(true)
    setDisplayGrade(false)
    setGhost(true)
    setCreditRoll(false)
    setMaxLevel(400)
    setLevel(0)
end

--[[ This Function gets called every frame --]]
function update()
end

function advanceLevel(amount, level, lineCount)

    -- Calculate Score
    setScore(getScore() + (((level + lineCount) / 4) + getSoftFrames()) * lineCount) -- TODO combo
    setSoftFrames(0)

    -- advance level if not on a level stop
    if ((level % 100 ~= 99 and level ~= 498) or lineCount ~= 0) then
        setLevel(amount + level + lineCount)
    end

    if (getLevel() >= 400) then
        setSectionLevel(499)
    else
        setSectionLevel(math.floor(getLevel() / 100 + 1) * 100)
    end

    -- Set gravity based on level
	if     (level < 30)  then setGravity(4)
	elseif (level < 40)  then setGravity(5)
	elseif (level < 80)  then setGravity(7)
	elseif (level < 100)  then setGravity(9)
	elseif (level < 140)  then setGravity(12)
	elseif (level < 180)  then setGravity(16)
	elseif (level < 200)  then setGravity(22)
	elseif (level < 240)  then setGravity(32)
	elseif (level < 280) then setGravity(45)
	elseif (level < 300) then setGravity(67)
	elseif (level < 340) then setGravity(99)
	elseif (level < 380) then setGravity(112)
	elseif (level < 400) then setGravity(151)
	elseif (level < 410) then setGravity(235)
	elseif (level < 420) then setGravity(374)
	elseif (level < 430) then setGravity(604)
	elseif (level < 440) then setGravity(1000)
	elseif (level < 450) then setGravity(1692)
	elseif (level < 460) then setGravity(2926)
	else setGravity(5120)
    end


    if (level >= 500 and getCreditRoll() == false) then
        setInvis(true)
        setCreditRoll(true)
    end

    if (level >= 499) then
        setLevel(499)
        if (getCreditRoll() == false) then
            setInvis(true)
            setCreditRoll(true)
            setLockDelay(40)
        end
    end

    bgIdx = 0

    if (getSectionLevel() > 100 and getSectionLevel() ~= 999) then
        bgIdx = (getSectionLevel() - 100) / 100
    end

    --setBackground(bgIdx)
end
