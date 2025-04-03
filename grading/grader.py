import argparse
import json
import os
import shutil
import subprocess
import sys
import re
from datetime import datetime
from types import SimpleNamespace
from typing import Callable

import github
import csv
import requests
import zipfile


def get_github() -> github.Github:
    env = "GITHUB_CLIENT"
    if env in globals():
        return globals()[env]

    access_token = os.environ.get("GITHUB_TOKEN", None)
    auth = github.Auth.Token(access_token) if access_token is not None else None
    gh = github.Github(auth=auth)
    globals()[env] = gh
    return gh


grade_fn = Callable[[str, str], dict]


def get_assignments() -> list[tuple[str, grade_fn]]:
    assignments = []
    if args.assignments:
        assignments = set(args.assignments)
        assignments = [(a, ASSIGNMENTS[a]) for a in assignments]
    elif args.all_assignments:
        assignments = list(ALL_ASSIGNMENTS.items())
    return assignments


def get_students():
    if args.students:
        return list(args.students)
    elif args.all_students:
        with open('grading/students.csv') as f:
            rows = list(csv.reader(f))[1:]
        return [
            get_student_name(first, last)
            for [first, last, _] in rows
        ]


def cmake(repo_path, build_dir) -> bool:
    command = ['cmake', '-S', '-B', build_dir]

    result = subprocess.run(command, capture_output=True)
    if result.returncode != 0:
        with open(f'{repo_path}/cmake.err', 'wb') as f:
            f.write(result.stderr)
        print(f'cmake failed, see {repo_path}/cmake.err', file=sys.stderr)
        return False
    return True


def mkdir_replace(path, copy_from=None) -> str:
    if os.path.exists(path):
        shutil.rmtree(path)
    if copy_from is not None:
        shutil.copytree(copy_from, path)
    else:
        os.mkdir(path)
    return path


def write_file(path, content):
    with open(path, mode="w") as f:
        f.write(content)

def read_file(path, default=''):
    try:
        with open(path) as f:
            return f.read()
    except IOError as _:
        return default


def utest(student, target):
    command = [f'grading/repos/{student}/build/{target}', '--list-tests']
    if args.verbose:
        print(f'running command {command}')
    result = subprocess.run(command, capture_output=True)
    tests = list(result.stdout.decode().splitlines())

    test_results = {
        'tests': {},
        'passed': 0,
        'failed': 0,
        'total': len(tests),
    }
    for test in tests:
        command = [f'grading/repos/{student}/build/{target}', f'--filter={test}']
        if args.verbose:
            print(f'running command {command}')
        try:
            result = subprocess.run(command, capture_output=True, timeout=30)
        except subprocess.TimeoutExpired:
            result = SimpleNamespace(
                returncode=1,
                stdout=b"",
                stderr=b"timed out",
            )
        if 0 == result.returncode:
            test_results['tests'][test] = 'passed'
            test_results['passed'] += 1
        else:
            try:
                test_results['tests'][test] = {"stdout": result.stdout.decode(), "stderr": result.stderr.decode()}
            except UnicodeDecodeError as e:
                test_results['tests'][test] = {"stdout": "", "stderr": str(e)}
            test_results['failed'] += 1

    return test_results


def gtest(student, target):
    command = [f'grading/repos/{student}/build/{target}', '--gtest_list_tests']
    if args.verbose:
        print(f'running command {command}')
    result = subprocess.run(command, capture_output=True)

    label = ""
    tests = []
    for line in result.stdout.decode().splitlines():
        if line.startswith("  "):
            tests.append(f"{label}{line[2:]}")
        else:
            label = line

    test_results = {
        'tests': {},
        'passed': 0,
        'failed': 0,
        'total': len(tests),
    }
    for test in tests:
        command = [f'grading/repos/{student}/build/{target}', f'--gtest_filter={test}']
        if args.verbose:
            print(f'running command {command}')
        try:
            result = subprocess.run(command, capture_output=True, timeout=30)
        except subprocess.TimeoutExpired:
            result = SimpleNamespace(
                returncode=1,
                stdout=b"",
                stderr=b"timed out",
            )
        if 0 == result.returncode:
            test_results['tests'][test] = 'passed'
            test_results['passed'] += 1
        else:
            try:
                test_results['tests'][test] = {"stdout": result.stdout.decode(), "stderr": result.stderr.decode()}
            except UnicodeDecodeError as e:
                test_results['tests'][test] = {"stdout": "", "stderr": str(e)}
            test_results['failed'] += 1

    return test_results


def setup_cmake_for_student(student_name) -> dict:
    build_dir = f"grading/repos/{student_name}/build"
    if os.path.exists(build_dir):
        return {}

    command = [
        "cmake",
        "-B", build_dir,
        "-D", f"STUDENT_DIR=repos/{student_name}/repo",
        "grading/",
    ]
    if args.verbose:
        print(f"running command {command}")

    result = subprocess.run(command, capture_output=True)
    if result.returncode != 0:
        return {"error": result.stderr.decode()}
    return {}


def make_target(student, target):
    command = [
        "make",
        "-j", "6",
        "-C", f"grading/repos/{student}/build",
        target
    ]
    if args.verbose:
        print(f"running command {command}")

    result = subprocess.run(command, capture_output=True)
    if result.returncode != 0:
        stderr = result.stderr.decode()
        if args.verbose:
            print(f"make failed: {stderr!r}")
        return {"error": stderr}
    return {}


def grade_bit_tak_toe(student):
    result = setup_cmake_for_student(student)
    if result: return {"btt": result}

    result = make_target(student, "btt_grading_tests")
    if result: return {"btt": result}

    result = utest(student, "btt_grading_tests")
    return {"btt": result}


def grade_asm(student):
    result = setup_cmake_for_student(student)
    if result: return {"asm": result}

    result = make_target(student, "asm_grading_tests")
    if result: return {"asm": result}

    result = gtest(student, "asm_grading_tests")
    return {"asm": result}


def grade_emulator(student):
    result = setup_cmake_for_student(student)
    if result: return {"emulator": result}

    result = make_target(student, "emulator_grading_tests")
    if result: return {"emulator": result}

    result = gtest(student, "emulator_grading_tests")
    return {"emulator": result}


def grade_languages(student):
    zt = grade_zortran(student)
    fr = grade_firth(student)
    sea = grade_sea(student)
    return {**zt, **fr, **sea}


def grade_sea(student):
    result = setup_cmake_for_student(student)
    if result: return {"sea": result}

    result = make_target(student, "sea_grading_tests")
    if result: return {"sea": result}

    result = gtest(student, "sea_grading_tests")
    return {"sea": result}


def grade_firth(student):
    result = setup_cmake_for_student(student)
    if result: return {"firth": result}

    result = make_target(student, "firth_grading_tests")
    if result: return {"firth": result}

    result = gtest(student, "firth_grading_tests")
    return {"firth": result}


def grade_zortran(student):
    result = setup_cmake_for_student(student)
    if result: return {"zortran": result}

    result = make_target(student, "zortran_grading_tests")
    if result: return {"zortran": result}

    result = gtest(student, "zortran_grading_tests")
    return {"zortran": result}


def grade_assembly(student):
    homework_file = f"grading/repos/{student}/repo/assembly/homework/homework.asm"
    command = ["java", "-jar", "assembly/Mars4_5.jar", homework_file]
    if args.verbose:
        print(f"running command {command}")
    result = subprocess.run(command, capture_output=True)
    output = result.stdout.decode()

    with open("grading/testsets/assembly.txt") as f:
        expected = f.read()

    passed = 1 if expected == output else 0
    failed = 1 - passed
    return {
        "assembly": {
            "tests": {
                "homework": "passed" if expected == output else {
                    "stdout": output,
                    "stderr": "",
                },
            },
            "passed": passed,
            "failed": failed,
            "total": 1,
        }
    }

def during_this_semester(ts: datetime) -> bool:
    now = datetime.now(ts.tzinfo)
    if now.year != ts.year:
        return False

    spring = {1, 2, 3, 4, 5}  # jan-may
    fall = {8, 9, 10, 11, 12}  # aug-dec

    if now.month in spring:
        semester_months = spring
    elif now.month in fall:
        semester_months = fall
    else:
        raise Exception("semester checking failed")

    return ts.month in semester_months


def get_student_name(first, last) -> str:
    first = re.sub(r'\W', '', first)
    last = re.sub(r'\W', '', last)
    student_name = re.sub(r'\W+', '_', f'{first} {last}')
    return student_name


def accept_invites():
    gh = get_github()
    for invite in gh.get_user().get_invitations():
        if "366" not in invite.repository.name:
            print(f"skipping {invite.repository.full_name!r} cuz it ain't a 366 repo")
            continue

        if not during_this_semester(invite.created_at):
            if args.verbose:
                print(f"skipping {invite.repository.full_name!r} cuz it's old")
            continue

        if not args.dry_run:
            try:
                gh.get_user().accept_invitation(invite)
                if args.verbose:
                    print(f"accepted invite to {invite.repository.full_name!r}")
            except github.GithubException:
                if args.verbose:
                    print(f"failed to accept invite to {invite.repository.full_name!r}")
        else:
            print(f"accepted invite to {invite.repository.full_name!r} (not really)")


def clone_repos():
    if args.clean and os.path.exists('grading/repos/'):
        if not args.dry_run:
            if args.verbose:
                print('removing cloned repos')
            shutil.rmtree('grading/repos/')
        else:
            print('removing cloned repos (not really)')

    gh = get_github()
    gh_token = os.environ['GITHUB_TOKEN']

    if not os.path.exists("grading/repos/"):
        if not args.dry_run:
            if args.verbose:
                print("running 'mkdir grading/repos/'")
            os.mkdir("grading/repos/")
        else:
            print("running 'mkdir grading/repos/' (not really)")

    with open('grading/students.csv') as f:
        rows = list(csv.reader(f))[1:]

    for row in rows:
        student_name = get_student_name(row[0], row[1])

        if os.path.exists(f'grading/repos/{student_name}/repo'):
            if args.verbose:
                print(f'skipping {student_name!r}, repo already cloned')
            continue

        if not os.path.exists(f'grading/repos/{student_name}'):
            if not args.dry_run:
                if args.verbose:
                    print(f"running 'mkdir grading/repos/{student_name}'")
                os.mkdir(f'grading/repos/{student_name}')
            else:
                print(f"running 'mkdir grading/repos/{student_name}' (not really)")

        repo_url = row[2]
        if repo_url.startswith('git@'):
            repo_url = repo_url.removeprefix('git@')
            repo_url = repo_url.replace(':', '/')
        else:
            repo_url = repo_url.removeprefix('https://')

        gh_username = gh.get_user().login
        repo_url = f'https://{gh_username}:{gh_token}@{repo_url}'
        command = ['git', 'clone', repo_url, f'grading/repos/{student_name}/repo']
        if not args.dry_run:
            if args.verbose:
                print(f'running {command}')
            subprocess.run(command, check=True)
        else:
            print(f'running {command} (not really)')


LATEST_TESTSET = None
def update_testsets():
    now = datetime.now()
    season = 'spring' if now.month <= 6 else 'fall'
    repo_tag = f'msu/csci-366-{season}{now.year}'

    if repo_tag in subprocess.run(['git', 'remote', 'get-url', 'origin'], capture_output=True).stdout.decode():
        return

    global LATEST_TESTSET
    if LATEST_TESTSET is not None:
        return
    gh = get_github()
    repo = gh.get_repo(repo_tag)
    release = repo.get_release('testset')
    LATEST_TESTSET = release

    testset_version_file = f'grading/testsets/.version'
    testset_modified_date = read_file(testset_version_file)
    if testset_modified_date == str(release.last_modified_datetime) and not args.clean:
        return

    assets = {asset.name: asset.browser_download_url for asset in release.get_assets()}
    testset_file_url = assets['testsets.zip']
    if args.verbose:
        print(f"downloading testsets.zip from {testset_file_url!r}")
    file = requests.get(testset_file_url, stream=True)
    file.raise_for_status()
    target_file = "grading/testsets.zip"
    with open(target_file, "wb") as f:
        for chunk in file.iter_content():
            f.write(chunk)
    with zipfile.ZipFile(target_file, 'r') as zr:
        target = "grading/testsets"
        if os.path.exists(target):
            shutil.rmtree(target)
        os.mkdir(target)
        zr.extractall("grading")
    os.remove(target_file)
    write_file(testset_version_file, str(release.last_modified_datetime))


def grade():
    update_testsets()

    assignments = get_assignments()
    if args.verbose:
        names = [name for (name, _) in assignments]
        print(f'grading assignments: {", ".join(names)}')

    students = get_students()

    grades = []
    for student in students:
        if not os.path.exists(f'grading/repos/{student}'):
            if args.verbose:
                print(f"skipping {student!r}, directory doesn't exist")
            continue

        build_dir = f"grading/repos/{student}/build"
        if args.clean and os.path.exists(build_dir):
            if args.verbose:
                print(f"cleaning {build_dir!r}")
            shutil.rmtree(build_dir)

        results = {}
        print(f'grading {student!r}')
        for name, grade_fn in assignments:
            test_results = grade_fn(student)
            print(f"  graded {name!r}")
            results |= test_results

        summary = {}
        passed = 0
        total = 0
        for name, test_results in results.items():
            if 'passed' not in test_results:
                continue  # skip assembly
            passed += test_results['passed']
            total += test_results['total']

            if test_results['total'] == 0:
                test_grade = 100
            else:
                test_grade = 100 * test_results['passed'] / test_results['total']
            summary[name] = f"{test_grade:.2f}"

        student_grade = passed / total if total != 0 else 0
        summary['project'] = f"(unweighted) {100 * student_grade:.2f}"
        results['summary'] = summary

        results_json = json.dumps(results, indent=4)
        print(results_json)

        with open(f"grading/repos/{student}/results.json", "w") as f:
            f.write(results_json)

        grades.append((student, passed, total, student_grade))

    grade_file = None
    if args.to:
        grade_file = args.to
    elif args.all_students and args.all_assignments:
        grade_file = "grading/grades.csv"

    if grade_file is not None:
        with open(grade_file, "w") as f:
            w = csv.writer(f)
            w.writerow(('student', 'passed', 'total', 'grade'))
            w.writerows(grades)


ALL_ASSIGNMENTS = {
    'bit-tak-toe': grade_bit_tak_toe,
    'asm': grade_asm,
    'emulator': grade_emulator,
    'sea': grade_sea,
    'firth': grade_firth,
    'zortran': grade_zortran,
    'assembly': grade_assembly,
}

ASSIGNMENTS = {
    'bit-tak-toe': grade_bit_tak_toe,
    'asm': grade_asm,
    'emulator': grade_emulator,
    'lang': grade_languages,
    'sea': grade_sea,
    'firth': grade_firth,
    'zortran': grade_zortran,
    'assembly': grade_assembly,
}

if __name__ == "__main__":
    if not os.path.exists("SYLLABUS.md"):
        print("error: must be run in repo root")
        exit(1)

    parser = argparse.ArgumentParser("autograder", description="the autograder for 366")
    commands = parser.add_subparsers(title="subcommands", dest="command", required=True)

    parser_accept = commands.add_parser("accept-invites", help='accept all 366 invites')
    parser_accept.add_argument("-v", default=False, action='store_true', dest="verbose", help='print verbose output')
    parser_accept.add_argument("--dry-run", default=False, action='store_true',
                               help="print all invites that would be accepted, but don't actually accept them")

    parser_clone = commands.add_parser("clone-repos", help='clone all 366 repos in students.csv')
    parser_clone.add_argument("-v", default=False, action='store_true', dest="verbose", help='print verbose output')
    parser_clone.add_argument("--dry-run", default=False, action='store_true',
                              help="print all invites that would be accepted, but don't actually accept them")
    parser_clone.add_argument("--clean", default=False, action='store_true',
                              help='remove cloned repos and re-clone')

    parser_grade = commands.add_parser("grade", help='grade code')
    parser_grade.add_argument("--to", help='a file to write grades to')
    parser_grade.add_argument("--clean", default=False, action='store_true', help='clean build directory first')
    parser_grade.add_argument("-v", default=False, action='store_true', dest="verbose", help='print verbose output')
    group = parser_grade.add_mutually_exclusive_group(required=True)
    group.add_argument("--student", "-s", dest='students', action='append',
                       help="'first_last' student identifier (corresponds to repos/)")
    group.add_argument("--all-students", action='store_true', help='grade all students')
    group = parser_grade.add_mutually_exclusive_group(required=True)
    group.add_argument("--assignment", "-a", dest='assignments', action='append', choices=ASSIGNMENTS.keys(),
                       help='list of assignments to grade')
    group.add_argument("--all-assignments", action='store_true', help='grade all assignments')

    args = parser.parse_args()
    print(args)

    if args.command == "accept-invites":
        accept_invites()
    if args.command == "clone-repos":
        clone_repos()
    elif args.command == "grade":
        grade()
    else:
        parser.print_help()
        exit(1)
