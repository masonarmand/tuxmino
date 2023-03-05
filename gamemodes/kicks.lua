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
function advanceLevel(amount, level, lineCount)

    -- timings->score += (((*level + lines) / 4) + timings->linesSoftDropped) * lines * timings->prevComboScore;
    -- timings->linesSoftDropped = 0;


    -- advance level if not on a level stop
    if ((level % 100 ~= 99 and level ~= 998) or lineCount ~= 0) then
        setLevel(amount + level + lineCount)
    end

    -- Calculate Score
    if (lineCount > 0) then
        combo = (lineCount - 1) * 2
        addScore((math.ceil((getLevel() + lineCount) / 4) + getSoftFrames()) * lineCount * combo) -- TODO combo and bravo
    end
    setSoftFrames(0)

    if (getLevel() >= 900) then
        setSectionLevel(999)
    else
        setSectionLevel(math.floor(getLevel() / 100 + 1) * 100)
    end

    -- Set gravity based on level
	if     (level >= 100)  then setGravity(5120)
	else setGravity(4)
    end


    if (getSectionLevel() ~= 999) then
        bgIdx = 0

        if (getSectionLevel() > 100 and getSectionLevel() ~= 999) then
            bgIdx = (getSectionLevel() - 100) / 100
        end

        setBackground(bgIdx)
    else
        setBackground(9)
    end
end
