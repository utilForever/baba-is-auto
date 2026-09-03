"""
Copyright (c) 2020-2026 Chris Ohk

I am making my contributions/submissions to this project solely in our
personal capacity and am not conveying any rights to any intellectual
property of any third parties.
"""

import pyBaba

def test_rule_manager_basic():
	rule_manager = pyBaba.RuleManager()
	rule1 = pyBaba.Rule(pyBaba.Object([pyBaba.ObjectType.BABA]), pyBaba.Object([pyBaba.ObjectType.IS]), pyBaba.Object([pyBaba.ObjectType.YOU]))
	rule2 = pyBaba.Rule(pyBaba.Object([pyBaba.ObjectType.KEKE]), pyBaba.Object([pyBaba.ObjectType.IS]), pyBaba.Object([pyBaba.ObjectType.STOP]))
	rule_manager.AddRule(rule1)
	assert rule_manager.GetNumRules() == 1
	rule_manager.AddRule(rule2)
	assert rule_manager.GetNumRules() == 2
	rule_manager.RemoveRule(rule2)
	assert rule_manager.GetNumRules() == 1


def test_rule_manager_all_is_not_property_matches_icons():
    rule_manager = pyBaba.RuleManager()
    rule_manager.AddRule(
        pyBaba.Rule(
            pyBaba.Object([pyBaba.ObjectType.BABA]),
            pyBaba.Object([pyBaba.ObjectType.IS]),
            pyBaba.Object([pyBaba.ObjectType.YOU]),
        )
    )
    rule_manager.AddRule(
        pyBaba.Rule(
            pyBaba.Object([pyBaba.ObjectType.ALL]),
            pyBaba.Object([pyBaba.ObjectType.IS]),
            pyBaba.Object([pyBaba.ObjectType.YOU]),
            [],
            True,
        )
    )

    assert not rule_manager.HasProperty(
        [pyBaba.ObjectType.ICON_BABA], pyBaba.ObjectType.YOU
    )
    assert rule_manager.FindPlayer() == pyBaba.ObjectType.ICON_EMPTY
