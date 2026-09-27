"""
Copyright (c) 2020-2026 Chris Ohk

I am making my contributions/submissions to this project solely in our
personal capacity and am not conveying any rights to any intellectual
property of any third parties.
"""

import pyBaba

import pytest


@pytest.mark.parametrize("negative_first", [False, True])
def test_is_not_rule_identity_and_precedence(negative_first):
    def rule(noun, prop, negated=False):
        return pyBaba.Rule(
            pyBaba.Object([noun]), pyBaba.Object([pyBaba.ObjectType.IS]),
            pyBaba.Object([prop]), [], negated,
        )

    positive = rule(pyBaba.ObjectType.BABA, pyBaba.ObjectType.YOU)
    negative = rule(pyBaba.ObjectType.BABA, pyBaba.ObjectType.YOU, True)
    assert not positive.predicate_negated
    assert negative.predicate_negated
    assert positive != negative

    manager = pyBaba.RuleManager()

    for entry in ([negative, positive] if negative_first else [positive, negative]):
        manager.AddRule(entry)

    manager.AddRule(rule(pyBaba.ObjectType.BABA, pyBaba.ObjectType.PUSH))
    manager.AddRule(rule(pyBaba.ObjectType.KEKE, pyBaba.ObjectType.YOU))
    assert not manager.HasProperty([pyBaba.ObjectType.ICON_BABA], pyBaba.ObjectType.YOU)
    assert manager.HasProperty([pyBaba.ObjectType.BABA], pyBaba.ObjectType.PUSH)
    assert manager.HasProperty(
        [pyBaba.ObjectType.BABA, pyBaba.ObjectType.KEKE], pyBaba.ObjectType.YOU
    )
    assert manager.FindPlayer() == pyBaba.ObjectType.ICON_KEKE

    manager.RemoveRule(negative)
    assert manager.HasProperty([pyBaba.ObjectType.BABA], pyBaba.ObjectType.YOU)
    assert manager.FindPlayer() == pyBaba.ObjectType.ICON_BABA


def test_is_not_parser_exposes_predicate_negation():
    rules = pyBaba.Game("Resources/Maps/is_not_properties.txt").GetRuleManager()
    push_rules = rules.GetRules(pyBaba.ObjectType.PUSH)
    assert len(push_rules) == 2
    assert sorted(rule.predicate_negated for rule in push_rules) == [False, True]
    assert all(not rule.predicate_negated for rule in rules.GetRules(pyBaba.ObjectType.STOP))


def test_is_not_find_player_checks_each_subject():
    obj = pyBaba.ObjectType

    manager = pyBaba.RuleManager()
    manager.AddRule(pyBaba.Rule(
        pyBaba.Object([obj.BABA, obj.KEKE]), pyBaba.Object([obj.IS]),
        pyBaba.Object([obj.YOU]),
    ))
    manager.AddRule(pyBaba.Rule(
        pyBaba.Object([obj.BABA]), pyBaba.Object([obj.IS]),
        pyBaba.Object([obj.YOU]), [], True,
    ))
    assert manager.FindPlayer() == obj.ICON_KEKE


@pytest.mark.parametrize("negative_first", [False, True])
def test_is_not_all_overrides_individual_subjects(negative_first):
    obj = pyBaba.ObjectType
    manager = pyBaba.RuleManager()

    positive = pyBaba.Rule(
        pyBaba.Object([obj.BABA]), pyBaba.Object([obj.IS]), pyBaba.Object([obj.YOU]),
    )
    negative = pyBaba.Rule(
        pyBaba.Object([obj.ALL]), pyBaba.Object([obj.IS]),
        pyBaba.Object([obj.YOU]), [], True,
    )

    for rule in ([negative, positive] if negative_first else [positive, negative]):
        manager.AddRule(rule)
    assert not manager.HasProperty([obj.BABA], obj.YOU)
    assert not manager.HasProperty([obj.ICON_BABA], obj.YOU)
    assert manager.FindPlayer() == obj.ICON_EMPTY

    manager.RemoveRule(negative)
    assert manager.HasProperty([obj.BABA], obj.YOU)
    assert manager.FindPlayer() == obj.ICON_BABA

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
