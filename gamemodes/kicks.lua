--[
--
-- File: master.lua
-- ---------------
-- Author: Mason Armand
-- Contributors:
--
--]

--[ This Function gets called once upon selecting the game mode --]
function start()
    -- Initialization and backgrounds
    initGameMode("kick testing", 255, 0, 255) -- Name and frame color (r, g, b)
    initBackgrounds(10) -- Number of backgrounds
    for i = 0, 9, 1 do
        initBgIdx(i, "res/bg/gradeMaster1/0"..i..".png")
    end
    setBackground(0)
    setPieceTint(150, 150, 150, 255)
    setPlayfieldColor(5, 0, 10, 220)

    setDAS(16)
    setARR(1)
    setARE(30)
    setLineARE(0)
    setLockDelay(30)
    setClearSpeed(41)

    setGravity(4)
    setPreview(3)
    setBagRetry(4)
    setGradeType(0)
    setGrade(0)
    setScore(0)
    setSevenBag(false)
    setHold(true)
    setSonicDrop(true)
    setExtraKicks(true)
    set3D(false)
    setDrawNextBg(true)
    setBig(false)
    setInvis(false)
    setDisplayGrade(false)
    setGhost(true)
    setCreditRoll(false)
    setClearOnCredits(false)
    setMaxLevel(900)
    setSectionLevel(100)
    setLevel(0)
    setNextScore(400)
end

--[ This Function gets called every frame --]
function update()
end

--[ This Function gets called every time a piece lands --]
function advanceLevel(amount, lineCount)

    setLevel(amount + getLevel() + lineCount)

    -- Set gravity based on level
	if (getLevel() >= 100) then
        setGravity(5120)
	else
        setGravity(4)
    end
end
